#include "AssemblyPlayerController.h"
#include "GameFramework/FloatingPawnMovement.h"

AAssemblyPlayerController::AAssemblyPlayerController()
{
	// 1. 默认开启鼠标交互（SOP / 拆装仿真必备）
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AAssemblyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 2. 初始化输入模式：显示鼠标且不锁定在视口中心
	SetupAssemblyInputMode();

	APawn* CurrentPawn = GetPawn();
	if (CurrentPawn)
	{
		CurrentPawn->SetActorEnableCollision(false);
		if (UFloatingPawnMovement* MoveCmp = Cast<UFloatingPawnMovement>(CurrentPawn->GetMovementComponent())) {
			MoveCmp->MaxSpeed = 50;
		}
	}
}

void AAssemblyPlayerController::SetupAssemblyInputMode()
{
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AAssemblyPlayerController::SetDraggingState(bool bIsDragging)
{
	bIsDraggingObject = bIsDragging;

	// 【核心逻辑】利用 UE 原生 API 一键锁定/解锁视角
	SetIgnoreLookInput(bIsDragging);

	if (bIsDragging)
	{
		// 开始拖拽：确保鼠标处于解放状态，避免被视口捕获隐藏
		SetupAssemblyInputMode();
	}
}

void AAssemblyPlayerController::AddPitchInput(float Val)
{
	// 拖拽零部件时截断相机的上下旋转
	if (bIsDraggingObject) return;

	Super::AddPitchInput(Val);
}

void AAssemblyPlayerController::AddYawInput(float Val)
{
	// 拖拽零部件时截断相机的左右旋转
	if (bIsDraggingObject) return;

	Super::AddYawInput(Val);
}