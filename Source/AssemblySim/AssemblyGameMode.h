// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Data/AssemblyTypes.h"
#include "AssemblyGameMode.generated.h"

// 声明 SOP 步骤变动委托 (用于通知 UI 界面更新指导文本与进度)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSOPStepChanged, const FAssemblyStepData&, NewStepData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSOPStepFailed, const FText&, ErrorReason, float, PenaltyScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSOPCompleted);

UCLASS()
class AAssemblyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAssemblyGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyGameMode")
	UDataTable* SOPDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyGameMode")
	bool bIsExamMode = false;

	/** 获取当前正在执行的步骤数据 */
	UFUNCTION(BlueprintPure, Category = "AssemblyGameMode")
	bool GetCurrentStepData(FAssemblyStepData& OutStepData) const;

	UFUNCTION(BlueprintCallable, Category = "AssemblyGameMode")
	bool OnCheckStep(EAssemblyAction Action, FName TargetA, FName TargetB);

	// ---------------- 委托事件 ----------------

	/** 当步骤成功推进时回调 */
	UPROPERTY(BlueprintAssignable, Category = "Assembly|Events")
	FOnSOPStepChanged OnSOPStepChanged;

	/** 当步骤操作失败 (错误放置/顺序错误) 时回调 */
	UPROPERTY(BlueprintAssignable, Category = "Assembly|Events")
	FOnSOPStepFailed OnSOPStepFailed;

	/** 当整套 SOP 所有步骤全部完成时回调 */
	UPROPERTY(BlueprintAssignable, Category = "Assembly|Events")
	FOnSOPCompleted OnSOPCompleted;

protected:
	/** 当前关卡所有步骤缓存 */
	TArray<FAssemblyStepData> SOPSteps;

	/** 当前游标指针 */
	int32 CurrentStepIndex = 0;


private:
	/** 初始化关卡 SOP 数据流 */
	void InitSOPSteps();

	/** 检查前置依赖步骤 */
	bool CheckPrerequisites(const TArray<int32>& PrerequisiteIndexes, FString& OutMissingStep) const;
};



