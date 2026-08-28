#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AssemblyPlayerController.generated.h"

/**
 * 工业装配仿真专用的 PlayerController
 * 统一管理鼠标光标模式、视角锁定以及拖拽状态
 */
UCLASS()
class ASSEMBLYSIM_API AAssemblyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAssemblyPlayerController();

	/** 切换拖拽状态：拖拽零部件时自动屏蔽视角旋转并设置 InputMode */
	UFUNCTION(BlueprintCallable, Category = "AssemblyPlayerController")
	void SetDraggingState(bool bIsDragging);

	/** 当前是否处于拖拽零部件状态 */
	UFUNCTION(BlueprintPure, Category = "AssemblyPlayerController")
	bool IsDraggingObject() const { return bIsDraggingObject; }

protected:
	virtual void BeginPlay() override;

	// 重写视角旋转输入，在拖拽零部件时截断相机的 Pitch/Yaw 响应
	virtual void AddPitchInput(float Val) override;
	virtual void AddYawInput(float Val) override;

private:
	/** 全局拖拽标志位 */
	bool bIsDraggingObject = false;

	/** 设置装配仿真标准的 GameAndUI 输入模式 */
	void SetupAssemblyInputMode();
};