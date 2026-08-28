#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AssemblyGameInstance.generated.h"

/**
 * 工业装配仿真全局 GameInstance
 * 负责跨关卡数据存储、SOP 流程配置以及全局状态管理
 */
UCLASS()
class ASSEMBLYSIM_API UAssemblyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UAssemblyGameInstance();

	virtual void Init() override;

	// ---------------- Global Data Stores ----------------

	/** 当前选中的装配任务 ID (跨关卡传递) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssemblyGameInstance")
	FString CurrentTaskID;

	/** 当前学员/操作员 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssemblyGameInstance")
	FString CurrentUserID;

	/** 全局装配得分/扣分统计 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssemblyGameInstance")
	float TotalScore = 100.0f;

	/** 记录步骤耗时 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssemblyGameInstance")
	float TotalElapsedTime = 0.0f;

	// ---------------- Global Helper APIs ----------------

	/** 重置当前考核数据 */
	UFUNCTION(BlueprintCallable, Category = "AssemblyGameInstance")
	void ResetSessionData();
};