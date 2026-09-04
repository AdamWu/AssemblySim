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

    SnapAnchorComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SnapAnchor"));
    SnapAnchorComponent->SetupAttachment(RootComponent);

}

void AAssemblyToolBase::BeginPlay()
{
    Super::BeginPlay();
}


void AAssemblyToolBase::AttachToNode(AAssemblyFastenerNode* Node)
{
    UE_LOG(LogTemp, Log, TEXT("AAssemblyToolBase::AttachToNode %s->%s"), *GetName(), *Node->GetName());

    AttachToComponent(Node->SnapAnchorComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    FTransform ToolAnchorLocal = SnapAnchorComponent->GetRelativeTransform();
    SetActorRelativeTransform(ToolAnchorLocal.Inverse());

    AttachedNode = Node;
    AttachedNode->UseTool(this);
}

void AAssemblyToolBase::DetachFromNode()
{
    UE_LOG(LogTemp, Log, TEXT("AAssemblyToolBase::DetachFromNode %s->%s"), *GetName(), *AttachedNode->GetName());

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    AttachedNode->ReleaseTool();
    AttachedNode = nullptr;
}

void AAssemblyToolBase::OnClicked()
{
    AttachedNode->Fasten();
}