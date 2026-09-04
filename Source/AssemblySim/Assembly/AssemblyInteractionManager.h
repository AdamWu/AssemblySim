// AssemblyInteractionManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputActionValue.h"
#include "AssemblyInteractionManager.generated.h"

class AAssemblyNodeBase;
class AAssemblyFastenerNode;
class UAssemblySlotComponent;
class AAssemblyToolBase;
class AAssemblyPlayerController;

UCLASS()
class ASSEMBLYSIM_API AAssemblyInteractionManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AAssemblyInteractionManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 当前按住并拖拽中的 Node 节点
	UPROPERTY()
	AActor* HeldActor = nullptr;

	bool HeldActorDetachLocked = true;
	bool HeldActorDetachTriggered = false;

	FVector DragActorLocation;
	float DragBaseZ = 0.0f;
	float DragZ = 0.0f;
	FVector DragOrigin;
	FVector DragOffset;

	UFUNCTION(BlueprintCallable, Category = "Assembly")
	void OnMouseLeftPressed();

	UFUNCTION(BlueprintCallable, Category = "Assembly")
	void OnMouseLeftReleased();

private:
	AAssemblyPlayerController* PC = nullptr;

	// 射线检测鼠标选中的 AssemblyNodeBase
	AActor* GetNodeUnderCursor() const;

	// 检索与 Node 相交的 SlotComponent
	UAssemblySlotComponent* FindOverlappingSlot(AAssemblyNodeBase* Node) const;

	// 检索与 Node 相交的 FastenerNode
	AAssemblyFastenerNode* FindOverlappingFastenerNode(AAssemblyToolBase* Node) const;
};