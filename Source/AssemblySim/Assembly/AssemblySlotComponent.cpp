#include "AssemblySlotComponent.h"
#include "AssemblyNodeBase.h"
#include "AssemblyFastenerNode.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

UAssemblySlotComponent::UAssemblySlotComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    //USceneComponent* RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    TriggerZone = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerZone"));
    TriggerZone->SetupAttachment(this);
    TriggerZone->SetCollisionProfileName(TEXT("Trigger"));

    PreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
    PreviewMeshComponent->SetupAttachment(this);
    PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PreviewMeshComponent->SetVisibility(false);
    
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("Material'/Game/Materials/M_Preview'"));
    if (MaterialFinder.Succeeded())
    {
        PreviewMaterial = MaterialFinder.Object;
    }

}

void UAssemblySlotComponent::BeginPlay()
{
    Super::BeginPlay();


    if (bIsLockPosition) SetUsingAbsoluteLocation(true);
    if (bIsLockRotation) SetUsingAbsoluteRotation(true);
    if (bIsLockScale) SetUsingAbsoluteScale(true);

    // 初始自动绑定
    if (OccupiedNode)
    {
        AAssemblyNodeBase* Node = OccupiedNode;
        OccupiedNode = nullptr;
        Node->AttachToSlot(this);
        if (AAssemblyFastenerNode* Fastner = Cast<AAssemblyFastenerNode>(Node)) {
            Fastner->bIsClosed = true;
        }
        Node->UpdateChildrenAssemblyStatus();
    }

    if (TriggerZone)
    {
        TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &UAssemblySlotComponent::OnTriggerBeginOverlap);
        TriggerZone->OnComponentEndOverlap.AddDynamic(this, &UAssemblySlotComponent::OnTriggerEndOverlap);
    }

    if (PreviewMeshComponent)
    {
        PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PreviewMeshComponent->SetVisibility(false);
    }

    if (PreviewMaterial && PreviewMeshComponent)
    {
        PreviewMeshComponent->SetMaterial(0, PreviewMaterial);
    }
}

bool UAssemblySlotComponent::CanAccept(const AAssemblyNodeBase* Node) const
{
    if (OccupiedNode || !Node) return false;

    // 校验 Tag 是否匹配，且确保部件没有被锁死在别处
    return Node->NodeTag.MatchesTagExact(AcceptNodeTag) && !Node->ParentSlot;
}

void UAssemblySlotComponent::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AAssemblyNodeBase* Node = Cast<AAssemblyNodeBase>(OtherActor);
    if (!Node) return;

    if (CanAccept(Node))
    {
        SetSlotHovered(true, Node);
    }

    UE_LOG(LogTemp, Log, TEXT("OnTriggerBeginOverlap %s"), *Node->NodeTag.ToString());
}

void UAssemblySlotComponent::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    AAssemblyNodeBase* Node = Cast<AAssemblyNodeBase>(OtherActor);
    if (!Node) return;
    
    SetSlotHovered(false, Node);

    UE_LOG(LogTemp, Log, TEXT("OnTriggerEndOverlap %s"), *Node->NodeTag.ToString());
}

void UAssemblySlotComponent::SetSlotHovered(bool bHovered, AAssemblyNodeBase* Node)
{
    if (OccupiedNode)
    {
        if (PreviewMeshComponent) PreviewMeshComponent->SetVisibility(false);
        return;
    }

    if (PreviewMeshComponent)
    {
        // 若插槽未预设 StaticMesh，可根据靠近部件的 Mesh 动态赋值
        if (bHovered && Node && !PreviewMeshComponent->GetStaticMesh())
        {
            if (UStaticMeshComponent* PartMeshComp = Node->FindComponentByClass<UStaticMeshComponent>())
            {
                PreviewMeshComponent->SetWorldScale3D(PartMeshComp->GetComponentScale());
                PreviewMeshComponent->SetStaticMesh(PartMeshComp->GetStaticMesh());
            }
        }

        PreviewMeshComponent->SetVisibility(bHovered);
    }

    OnSlotHoverChanged.Broadcast(bHovered, Node);
}

bool UAssemblySlotComponent::OccupySlot(AAssemblyNodeBase* Node, bool bSilent)
{
    if (!Node || OccupiedNode) return false;

    UE_LOG(LogTemp, Log, TEXT("OccupySlot %s"), *AcceptNodeTag.ToString());

    OccupiedNode = Node;

    SetSlotHovered(false, nullptr);
    SetSlotActive(false);

    AAssemblyNodeBase* OwnerNode = Cast<AAssemblyNodeBase>(GetOwner());
    if (OwnerNode)
    {
        OwnerNode->UpdateChildrenAssemblyStatus();
    }

    OnSlotOccupiedChanged.Broadcast(true);
    return true;
}

bool UAssemblySlotComponent::ClearSlot()
{
    if (!OccupiedNode) return false;

    UE_LOG(LogTemp, Log, TEXT("ClearSlot %s"), *AcceptNodeTag.ToString());

    OccupiedNode = nullptr;

    SetSlotActive(true);

    AAssemblyNodeBase* OwnerNode = Cast<AAssemblyNodeBase>(GetOwner());
    if (OwnerNode)
    {
        OwnerNode->UpdateChildrenAssemblyStatus();
    }

    OnSlotOccupiedChanged.Broadcast(false);
    return true;
}

void UAssemblySlotComponent::SetSlotActive(bool bActive)
{
    if (TriggerZone)
    {
        TriggerZone->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }
}