// AssemblyToolBase.cpp
#include "AssemblyToolBase.h"
#include "AssemblyNodeBase.h"
#include "AssemblyFastenerNode.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

AAssemblyToolBase::AAssemblyToolBase()
{
    PrimaryActorTick.bCanEverTick = true;

    DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
    SetRootComponent(DefaultRoot);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

}

void AAssemblyToolBase::BeginPlay()
{
    Super::BeginPlay();

    AnchorComponent = FindObject<USceneComponent>(this, TEXT("Anchor"));
}


bool AAssemblyToolBase::AttachToNode(AAssemblyFastenerNode* Node)
{
    if (AttachedNode) return false;

    UE_LOG(LogTemp, Log, TEXT("AAssemblyToolBase::AttachToNode %s->%s"), *GetName(), *Node->GetName());

    AttachToComponent(Node->SnapAnchorComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    FTransform ToolAnchorLocal = AnchorComponent->GetRelativeTransform();
    SetActorRelativeTransform(ToolAnchorLocal.Inverse());

    AttachedNode = Node;
    AttachedNode->UseTool(this);

    return true;
}

bool AAssemblyToolBase::DetachFromNode()
{
    if (!AttachedNode) return false;

    UE_LOG(LogTemp, Log, TEXT("AAssemblyToolBase::DetachFromNode %s->%s"), *GetName(), *AttachedNode->GetName());

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    AttachedNode->ReleaseTool();
    AttachedNode = nullptr;

    return true;
}

void AAssemblyToolBase::OnClicked()
{
    if (AttachedNode) {
        AttachedNode->Fasten();
    }
}