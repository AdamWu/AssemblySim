#include "AssemblyFastenerNode.h"
#include "AssemblyToolBase.h"
#include "Components/SphereComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"

AAssemblyFastenerNode::AAssemblyFastenerNode()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AAssemblyFastenerNode::BeginPlay()
{
	Super::BeginPlay();

	SnapAnchorComponent = FindObject<USceneComponent>(this, TEXT("SnapAnchor"));

	WidgetComponent = FindObject<UWidgetComponent>(this, TEXT("Widget"));
	WidgetComponent->SetVisibility(false);

	TriggerZone = FindObject<UShapeComponent>(this, TEXT("Trigger"));
	TriggerZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &AAssemblyFastenerNode::OnToolOverlapBegin);
	TriggerZone->OnComponentEndOverlap.AddDynamic(this, &AAssemblyFastenerNode::OnToolOverlapEnd);
}


bool AAssemblyFastenerNode::AttachToSlot(UAssemblySlotComponent* Slot)
{
	bool ret = Super::AttachToSlot(Slot);

	TriggerZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	return ret;
}


bool AAssemblyFastenerNode::DetachFromSlot()
{
	bool ret = Super::DetachFromSlot();

	TriggerZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	return ret;
}

bool AAssemblyFastenerNode::CanDetachNode() const
{
	bool ret = Super::CanDetachNode();
	return ret && !bIsFastened && !CurrentTool;
}

bool AAssemblyFastenerNode::CanAcceptTool(AAssemblyToolBase* Tool) const
{
	return !bIsFastened && CurrentTool == nullptr && Tool;
}

void AAssemblyFastenerNode::UseTool(AAssemblyToolBase* Tool)
{
	CurrentTool = Tool;
	SetHighlightEnabled(true);

	WidgetComponent->SetVisibility(true);
}

void AAssemblyFastenerNode::ReleaseTool()
{
	CurrentTool = nullptr;
	SetHighlightEnabled(false);

	WidgetComponent->SetVisibility(false);
}


void AAssemblyFastenerNode::OnToolOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                                bool bFromSweep, const FHitResult& SweepResult)
{
	AAssemblyToolBase* Tool = Cast<AAssemblyToolBase>(OtherActor);
	if (Tool && CanAcceptTool(Tool))
	{
		// 触碰高亮提示
		MeshComponent->SetRenderCustomDepth(true);
		// 设置 Stencil 值（可以用不同数字代表不同颜色的外发光，如 1=绿色高亮, 2=黄色预警）
		MeshComponent->SetCustomDepthStencilValue(1);
	}
}

void AAssemblyFastenerNode::OnToolOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 取消高亮
	MeshComponent->SetRenderCustomDepth(false);
}