#include "AssemblyNodeBase.h"
#include "AssemblySlotComponent.h"
#include "Components/StaticMeshComponent.h"

AAssemblyNodeBase::AAssemblyNodeBase()
{
    PrimaryActorTick.bCanEverTick = false;

    DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
    SetRootComponent(DefaultRoot);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(DefaultRoot);
}

void AAssemblyNodeBase::BeginPlay()
{
    Super::BeginPlay();

    // 收集挂载在自身 Actor 上的所有插槽组件
    GetComponents<UAssemblySlotComponent>(ChildSlots);

    // 初始状态刷新
    UpdateChildrenAssemblyStatus();
}


bool AAssemblyNodeBase::AttachToSlot(UAssemblySlotComponent* Slot)
{
    if (Slot->OccupiedNode) return false;

    UE_LOG(LogTemp, Log, TEXT("AttachToSlot %s->%s"), *NodeTag.ToString(), *Slot->AcceptNodeTag.ToString());

    Slot->OccupySlot(this);

    AttachToComponent(Slot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    ParentSlot = Slot;

    RefreshChildSlotsActivation();
    return true;
}

bool AAssemblyNodeBase::DetachFromSlot()
{
    if (!ParentSlot) return false;

    UE_LOG(LogTemp, Log, TEXT("DetachFromSlot %s->%s"), *NodeTag.ToString(), *ParentSlot->AcceptNodeTag.ToString());

    ParentSlot->ClearSlot();

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    ParentSlot = nullptr;

    RefreshChildSlotsActivation();
    return true;
}

bool AAssemblyNodeBase::CanDetachNode() const
{
    // 根节点禁止被拆走
    if (bIsRootNode) return false;

    // Rule1: 父节点限制
    if (ParentSlot && ParentSlot->bIsLocked) return false;

    // Rule2: 子节点限制
    for (const UAssemblySlotComponent* Slot : ChildSlots)
    {
        if (Slot && Slot->OccupiedNode)
        {
            return false;
        }
    }

    return true;
}

void AAssemblyNodeBase::UpdateChildrenAssemblyStatus()
{
    bool bAllChildrenReady = true;

    // 遍历检查自身所有的子插槽
    for (const UAssemblySlotComponent* Slot : ChildSlots)
    {
        if (!Slot) continue;

        // 如果插槽没被占用，或者插槽上的部件没有彻底锁死 (Secured)
        if (!Slot->OccupiedNode || !Slot->OccupiedNode->IsCompleted())
        {
            bAllChildrenReady = false;
            break;
        }
    }

    bIsAllChildrenAssembled = bAllChildrenReady;

    // 只要盖子被装上了 (bIsMounted)，即使螺丝还没拧，也开启盖子上的螺丝孔插槽
    RefreshChildSlotsActivation();

    // 向上递归通知上层节点
    if (ParentSlot && ParentSlot->GetOwner())
    {
        if (AAssemblyNodeBase* ParentNode = Cast<AAssemblyNodeBase>(ParentSlot->GetOwner()))
        {
            ParentNode->UpdateChildrenAssemblyStatus();
        }
    }
}

void AAssemblyNodeBase::RefreshChildSlotsActivation()
{
    // 只有当自身处于“已吸附/已安装”或“本身就是根节点”时，才激活自身的子插槽触发区
    const bool bShouldActivateSlots = bIsRootNode || ParentSlot;

    for (UAssemblySlotComponent* Slot : ChildSlots)
    {
        if (Slot)
        {
            // 如果插槽已经被占用了，保持关闭；只有空闲插槽才根据激活状态开启
            if (Slot->OccupiedNode)
            {
                Slot->SetSlotActive(false);
            }
            else
            {
                Slot->SetSlotActive(bShouldActivateSlots);
            }
        }
    }
}