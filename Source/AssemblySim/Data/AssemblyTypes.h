#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AssemblyTypes.generated.h"

/** 动作类型 */
UENUM(BlueprintType)
enum class EAssemblyAction : uint8
{
	Click          UMETA(DisplayName = "点击"),
	Snap           UMETA(DisplayName = "A 碰 B (吸附/放置)")
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
	EAssemblyAction Action = EAssemblyAction::Click;

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