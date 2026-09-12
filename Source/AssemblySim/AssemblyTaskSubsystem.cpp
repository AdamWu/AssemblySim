#include "AssemblyTaskSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "AssemblyGameMode.h"

void UAssemblyTaskSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
}

void UAssemblyTaskSubsystem::Deinitialize()
{
}

void UAssemblyTaskSubsystem::InitSOPSteps(UDataTable* DataTable)
{
	CurrentStepIndex = 0;

	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitSOPSteps: DataTable is null"));
		return;
	}

	if (DataTable->RowStruct != FAssemblyStepData::StaticStruct())
	{
		UE_LOG(LogTemp, Error, TEXT("InitSOPSteps: RowStruct mismatch!"));
		return;
	}

	// 遍历所有行
	static const FString ContextString(TEXT("InitSOPSteps"));
	TArray<FAssemblyStepData*> AllRows;
	DataTable->GetAllRows<FAssemblyStepData>(ContextString, AllRows);
	for (const FAssemblyStepData* RowPtr : AllRows)
	{
		if (RowPtr)
		{
			SOPSteps.Add(*RowPtr);
		}
	}

	// 按 StepIndex 排序（保证顺序，即使 CSV 中顺序乱）
	Algo::Sort(SOPSteps, [](const FAssemblyStepData& A, const FAssemblyStepData& B)
		{
			return A.StepIndex < B.StepIndex;
		});

	// 广播第一步步骤更新，驱动 UI 显示提示
	FAssemblyStepData FirstStep;
	if (GetCurrentStepData(FirstStep))
	{
		OnSOPStepChanged.Broadcast(FirstStep);
	}
}

void UAssemblyTaskSubsystem::InitEntityConfig(UDataTable* DataTable)
{
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitEntityConfig: DataTable is null"));
		return;
	}

	if (DataTable->RowStruct != FAssemblyEntityData::StaticStruct())
	{
		UE_LOG(LogTemp, Error, TEXT("InitEntityConfig: RowStruct mismatch!"));
		return;
	}

	static const FString ContextString(TEXT("InitEntityConfig"));
	TArray<FAssemblyEntityData*> AllRows;
	DataTable->GetAllRows<FAssemblyEntityData>(ContextString, AllRows);
	for (const FAssemblyEntityData* RowPtr : AllRows)
	{
		if (RowPtr)
		{
			EntityConfigs.Add(RowPtr->TagName, *RowPtr);
		}
	}
}

bool UAssemblyTaskSubsystem::GetCurrentStepData(FAssemblyStepData& OutStepData) const
{
	if (SOPSteps.IsValidIndex(CurrentStepIndex))
	{
		OutStepData = SOPSteps[CurrentStepIndex];
		return true;
	}
	return false;
}

bool UAssemblyTaskSubsystem::OnCheckStep(EAssemblyAction Action, AAssemblyToolBase* Tool, AAssemblyNodeBase* Node, UAssemblySlotComponent* Slot)
{
	if (bIsExamMode) return true;

	FAssemblyStepData CurrentStep;
	if (!GetCurrentStepData(CurrentStep))
	{
		UE_LOG(LogTemp, Warning, TEXT("current step is null"));
		return false;
	}

	FAssemblyLog Log;
	Log.Timestamp = FDateTime::Now();

	if (Tool) Log.ToolName = Tool->ToolID.ToString();
	if (Node) Log.NodeName = Node->NodeID.ToString();
	if (Slot) Log.SlotName = Slot->SlotID.ToString();

	FString TimestampStr = Log.Timestamp.ToString(TEXT("%H:%M:%S"));

	if (Action == EAssemblyAction::Attach || Action == EAssemblyAction::Detach)
	{
		FName TargetA = Tool ? Tool->ToolID : Node->NodeID;
		FName TargetB = Slot ? Slot->SlotID : Node->NodeID;

		FAssemblyEntityData EntityA = Tool ? EntityConfigs[Tool->ToolTag] : EntityConfigs[Node->NodeTag];
		FAssemblyEntityData EntityB = Slot ? EntityConfigs[Slot->SlotTag] : EntityConfigs[Node->NodeTag];
		Log.ActionName = EntityA.ActionNames[Action];
		
		if (Action == EAssemblyAction::Attach)
		{
			Log.FormatString = FString::Printf(TEXT("[%s] [%s] %s [%s]"), *TimestampStr, *EntityA.DisplayName, *Log.ActionName, *EntityB.DisplayName);
		}
		else if (Action == EAssemblyAction::Detach)
		{
			Log.FormatString = FString::Printf(TEXT("[%s] 从 [%s] %s [%s]"), *TimestampStr, *EntityB.DisplayName, *Log.ActionName, *EntityA.DisplayName);
		}

		if (CurrentStep.Action != Action || CurrentStep.TargetA != TargetA || CurrentStep.TargetB != TargetB)
		{
			FText ErrorMsg = FText::FromString(TEXT("零件或装配槽位不正确！"));
			UE_LOG(LogTemp, Warning, TEXT("error target！ %s"), *CurrentStep.Desc);
			return false;
		}

	}
	else if (Action == EAssemblyAction::Open || Action == EAssemblyAction::Close)
	{
		FName TargetA = Tool ? Tool->ToolID : Node->NodeID;

		FAssemblyEntityData EntityA = Tool ? EntityConfigs[Tool->ToolTag] : EntityConfigs[Node->NodeTag];
		Log.ActionName = EntityA.ActionNames[Action];
		
		if (Tool)
		{
			Log.FormatString = FString::Printf(TEXT("[%s] 使用 [%s] %s"), *TimestampStr, *EntityA.DisplayName, *Log.ActionName);
		}
		else
		{
			Log.FormatString = FString::Printf(TEXT("[%s] %s [%s]"), *TimestampStr, *Log.ActionName, *EntityA.DisplayName);
		}

		if (CurrentStep.Action != Action || CurrentStep.TargetA != TargetA)
		{
			FText ErrorMsg = FText::FromString(TEXT("零件或装配槽位不正确！"));
			UE_LOG(LogTemp, Warning, TEXT("error target！ %s"), *CurrentStep.Desc);
			return false;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("log: %s"), *Log.FormatString);

	CurrentStepIndex++;
	UE_LOG(LogTemp, Warning, TEXT("next step: CurrentStepIndex: %d"), CurrentStepIndex);

	// 检查是否全部装配完成
	if (CurrentStepIndex >= SOPSteps.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("all steps finished"));
		OnSOPCompleted.Broadcast();
	}
	else
	{
		// 广播下一步信息更新 UI
		FAssemblyStepData NextStep;
		GetCurrentStepData(NextStep);
		OnSOPStepChanged.Broadcast(NextStep);
	}

	return true;
}