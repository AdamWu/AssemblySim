#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AssemblyNodeBase.generated.h"

class UStaticMeshComponent;
class UAssemblySlotComponent;

UCLASS()
class ASSEMBLYSIM_API AAssemblyNodeBase : public AActor
{
    GENERATED_BODY()

public:
    AAssemblyNodeBase();

protected:
    virtual void BeginPlay() override;

public:
    // ==========================================
    // 基础组件
    // ==========================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    TObjectPtr<USceneComponent> DefaultRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    // ==========================================
    // 节点标识与状态
    // ==========================================

    /** 该部件的类型 Tag */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    FGameplayTag NodeTag;

    /** 是否为不可被抓取拆卸的根基座 (如工作台、发动机主缸体) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    bool bIsRootNode = false;

    /** 状态 1: 是否已吸附在插槽上 (Mounted) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    bool bIsAttached = false;

    /** 最终状态: 是否彻底锁死 (Secured) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    bool bIsFullySecured = false;

    /** 自身所有子节点/插槽是否全部组装锁死完毕 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    bool bIsAllChildrenAssembled = false;

    /** 所挂载的父插槽指针 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    TObjectPtr<UAssemblySlotComponent> ParentSlot = nullptr;

    /** 自身的子插槽列表 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    TArray<TObjectPtr<UAssemblySlotComponent>> ChildSlots;

    // ==========================================
    // 核心接口
    // ==========================================

    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    virtual bool AttachToSlot(UAssemblySlotComponent* Slot);

    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    virtual bool DetachFromSlot();

    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    virtual bool CanDetachNode() const;

    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    virtual bool IsSelfCompleted() { return bIsAttached; };

    /** 级联递归更新自身及父节点的组装完成状态 */
    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    void UpdateChildrenAssemblyStatus();

    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    void SetHighlightEnabled(bool bEnable) { MeshComponent->SetRenderCustomDepth(bEnable); };

protected:
    /** 根据安装状态自动开启/关闭身上插槽的碰撞触发区 */
    void RefreshChildSlotsActivation();
};