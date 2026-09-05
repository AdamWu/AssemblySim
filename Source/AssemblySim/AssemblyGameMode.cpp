// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssemblyGameMode.h"
#include "AssemblyPlayerController.h"
#include "UObject/ConstructorHelpers.h"

AAssemblyGameMode::AAssemblyGameMode()
{
	// 绑定自定义 PlayerController
	PlayerControllerClass = AAssemblyPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP_CameraController"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

}


void AAssemblyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	InitSOPSteps();
}


void AAssemblyGameMode::InitSOPSteps()
{
	CurrentStepIndex = 0;

	if (!SOPDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitSOPSteps: SOPDataTable is null"));
		return;
	}

	if (SOPDataTable->RowStruct != FAssemblyStepData::StaticStruct())
	{
		UE_LOG(LogTemp, Error, TEXT("InitSOPSteps: RowStruct mismatch!"));
		return;
	}

	// 遍历所有行
	static const FString ContextString(TEXT("SOPDataTableParsingContext"));
	TArray<FAssemblyStepData*> AllRows;
	SOPDataTable->GetAllRows<FAssemblyStepData>(ContextString, AllRows);
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

bool AAssemblyGameMode::GetCurrentStepData(FAssemblyStepData& OutStepData) const
{
	if (SOPSteps.IsValidIndex(CurrentStepIndex))
	{
		OutStepData = SOPSteps[CurrentStepIndex];
		return true;
	}
	return false;
}

bool AAssemblyGameMode::OnCheckStep(EAssemblyAction Action, FName TargetA, FName TargetB)
{
	if (bIsExamMode) return true;

	FAssemblyStepData CurrentStep;
	if (!GetCurrentStepData(CurrentStep))
	{
		UE_LOG(LogTemp, Warning, TEXT("current step is null"));
		return false;
	}

	// 1. 校验目标 Slot 与零件 ID 是否与当前步骤匹配
	if (CurrentStep.TargetA != TargetA || CurrentStep.TargetB != TargetB)
	{
		FText ErrorMsg = FText::FromString(TEXT("零件或装配槽位不正确！"));
		UE_LOG(LogTemp, Warning, TEXT("error target！ %s"), *CurrentStep.Desc);

		//OnSOPStepFailed.Broadcast(ErrorMsg, CurrentStep.ErrorPenaltyScore);
		return false;
	}


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