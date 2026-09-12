#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "AssemblyTypes.generated.h"

/** 动作类型 */
UENUM(BlueprintType)
enum class EAssemblyAction : uint8
{
	Attach   UMETA(DisplayName = "吸附"),   // A 连接到 B
	Detach   UMETA(DisplayName = "解绑"),   // A 从 B 断开
	Open     UMETA(DisplayName = "打开"),
	Close    UMETA(DisplayName = "关闭")
};

/** SOP 单步配置结构体 */
USTRUCT(BlueprintType)
struct FAssemblyStepData : public FTableRowBase
{
	GENERATED_BODY()

	/** 步骤序号 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SOP")
	int32 StepIndex = 0;

	/** UI 引导文本 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SOP")
	FString Desc = "";

	/** 动作类型 (Click / Snap) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SOP")
	EAssemblyAction Action = EAssemblyAction::Attach;

	/** 操作主体 ID (支持路径拼接，如 "Slot_Wheel_FL/Group_Bolts") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SOP")
	FName TargetA = NAME_None;

	/** 操作目标 ID (仅 Snap 动作使用) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SOP")
	FName TargetB = NAME_None;

	/** 
	 * 属性判断断言 (动态 key-value 条件判定)
	 * 示例 Key: "EquippedTool" -> Value: "Tool_Wrench_19"
	 * 示例 Key: "PowerState"   -> Value: "Off"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SOP")
	TMap<FName, FString> RequiredAttributes;
};

USTRUCT(BlueprintType)
struct FAssemblyEntityData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FGameplayTag TagName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FString DisplayName = TEXT("Unknown");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TMap<EAssemblyAction, FString> ActionNames;

	FAssemblyEntityData()
	{
		ActionNames.Add(EAssemblyAction::Attach, TEXT("安装"));
		ActionNames.Add(EAssemblyAction::Detach, TEXT("拆卸"));
		ActionNames.Add(EAssemblyAction::Open, TEXT("打开"));
		ActionNames.Add(EAssemblyAction::Close, TEXT("关闭"));
	}
};

USTRUCT(BlueprintType)
struct FAssemblyLog
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 StepIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	FString ActionName = "";

	UPROPERTY(BlueprintReadOnly)
	FString ToolName = "";

	UPROPERTY(BlueprintReadOnly)
	FString NodeName = "";

	UPROPERTY(BlueprintReadOnly)
	FString SlotName = "";

	UPROPERTY(BlueprintReadOnly)
	FString FormatString = "";

};