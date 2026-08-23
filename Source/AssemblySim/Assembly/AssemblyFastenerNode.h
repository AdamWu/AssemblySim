#pragma once

#include "CoreMinimal.h"
#include "AssemblyNodeBase.h"
#include "AssemblyFastenerNode.generated.h"

class USphereComponent;
class AAssemblyToolBase;
class UWidgetComponent;

UCLASS()
class ASSEMBLYSIM_API AAssemblyFastenerNode : public AAssemblyNodeBase
{
	GENERATED_BODY()

public:
	AAssemblyFastenerNode();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 组件与参数 ====================

	/** 工具靠近的触发区域与吸附对齐参照点 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	UShapeComponent* TriggerZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	USceneComponent* SnapAnchorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	UWidgetComponent* WidgetComponent;

	/** 匹配的工具 Tag（例如 "ScrewDriver", "Wrench"） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	FGameplayTag RequiredToolTag;

	/** 是否已彻底紧固完成 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	bool bIsFastened = false;

	/** 当前挂靠的工具指针（仅做引用，非 Child 组装节点） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	AAssemblyToolBase* CurrentTool = nullptr;


	virtual bool AttachToSlot(UAssemblySlotComponent* Slot) override;

	virtual bool DetachFromSlot() override;

	virtual bool CanDetachNode() const;

	virtual bool IsSelfCompleted() { return bIsFastened; };

	// ==================== 核心工具交互接口 ====================

	/** 1. 校验工具匹配度 */
	UFUNCTION(BlueprintCallable, Category = "AssemblyFastenerNode")
	bool CanAcceptTool(AAssemblyToolBase* Tool) const;

	UFUNCTION(BlueprintCallable, Category = "AssemblyFastenerNode")
	void UseTool(AAssemblyToolBase* Tool);

	UFUNCTION(BlueprintCallable, Category = "AssemblyFastenerNode")
	void ReleaseTool();

private:
	UFUNCTION()
	void OnToolOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
	                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
	                        bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnToolOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
	                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};