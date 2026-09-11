// ASQLiteManager.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/AssemblyTypes.h"
#include "Assembly/AssemblyNodeBase.h"
#include "Assembly/AssemblySlotComponent.h"
#include "Assembly/AssemblyToolBase.h"
#include "AssemblyTaskSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSOPStepChanged, const FAssemblyStepData&, NewStepData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSOPStepFailed, const FText&, ErrorReason, float, PenaltyScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSOPCompleted);

UCLASS()
class UAssemblyTaskSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

public:
	/** 初始化关卡 SOP 数据流 */
	void InitSOPSteps(UDataTable* SOPDataTable);

	void InitEntityConfig(UDataTable* DataTable);

	/** 获取当前正在执行的步骤数据 */
	UFUNCTION(BlueprintPure, Category = "AssemblyTaskSubsystem")
	bool GetCurrentStepData(FAssemblyStepData& OutStepData) const;

	UFUNCTION(BlueprintCallable, Category = "AssemblyTaskSubsystem")
	bool OnCheckStep(EAssemblyAction Action, AAssemblyToolBase* Tool, AAssemblyNodeBase* Node=nullptr, UAssemblySlotComponent* Slot=nullptr);

	/** 当步骤成功推进时回调 */
	UPROPERTY(BlueprintAssignable, Category = "AssemblyTaskSubsystem")
	FOnSOPStepChanged OnSOPStepChanged;

	/** 当步骤操作失败 (错误放置/顺序错误) 时回调 */
	UPROPERTY(BlueprintAssignable, Category = "AssemblyTaskSubsystem")
	FOnSOPStepFailed OnSOPStepFailed;

	/** 当整套 SOP 所有步骤全部完成时回调 */
	UPROPERTY(BlueprintAssignable, Category = "AssemblyTaskSubsystem")
	FOnSOPCompleted OnSOPCompleted;


public:
	bool bIsExamMode = false;

protected:
	TArray<FAssemblyStepData> SOPSteps;
	TMap<FGameplayTag, FAssemblyEntityData> EntityConfigs;

	int32 CurrentStepIndex = 0;

};