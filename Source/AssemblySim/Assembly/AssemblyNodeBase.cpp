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

    // 初始状态下，若自己还没被完全装好，先禁用身上的子插槽（比如盖子还没盖上，禁止插螺丝）
    RefreshChildSlotsActivation();
}


bool AAssemblyNodeBase::AttachToSlot(UAssemblySlotComponent* Slot)
{
    if (Slot->bIsOccupied) return false;

    UE_LOG(LogTemp, Log, TEXT("AttachToSlot %s->%s"), *NodeTag.ToString(), *Slot->AcceptNodeTag.ToString());

    Slot->OccupySlot(this);

    AttachToComponent(Slot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    ParentSlot = Slot;
    bIsAttached = true;

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
    bIsAttached = false;

    RefreshChildSlotsActivation();
    return true;
}

bool AAssemblyNodeBase::CanDetachNode() const
{
    // 根节点禁止被拆走
    if (bIsRootNode) return false;

    // 规则 2: 如果身上有子插槽被占用 (如盖子上还有螺丝没拆)，禁止拿走盖子！
    for (const UAssemblySlotComponent* Slot : ChildSlots)
    {
        if (Slot && Slot->bIsOccupied)
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
        if (!Slot->bIsOccupied || !Slot->CurrentOccupiedNode || !Slot->CurrentOccupiedNode->bIsFullySecured)
        {
            bAllChildrenReady = false;
            break;
        }
    }

    bIsAllChildrenAssembled = bAllChildrenReady;

    // 只要盖子被装上了 (bIsMounted)，即使螺丝还没拧，也开启盖子上的螺丝孔插槽
    RefreshChildSlotsActivation();

    // 向上递归通知上层节点 (例如：盖子装好了 $\rightarrow$ 通知发动机 $\rightarrow$ 发动机通知车身)
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
    const bool bShouldActivateSlots = bIsRootNode || bIsAttached;

    for (UAssemblySlotComponent* Slot : ChildSlots)
    {
        if (Slot)
        {
            // 如果插槽已经被占用了，保持关闭；只有空闲插槽才根据激活状态开启
            if (Slot->bIsOccupied)
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