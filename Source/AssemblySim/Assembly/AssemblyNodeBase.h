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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    FName NodeID;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
    {
        Super::PostEditChangeProperty(PropertyChangedEvent);

        FString TagString = NodeTag.ToString();
        TagString.RemoveFromStart(TEXT("Assembly."));
        TagString.ReplaceInline(TEXT("."), TEXT("_"));

        if (Index > 0) {
            NodeID = *FString::Printf(TEXT("%s_%02d"), *TagString, Index);
        }
        else {
            NodeID = *TagString;
        }
    }
#endif

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    FGameplayTag NodeTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    int32 Index = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    bool bIsRootNode = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    bool bIsClosed = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    bool bIsAllChildrenAssembled = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    TObjectPtr<UAssemblySlotComponent> ParentSlot = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyNodeBase")
    TArray<TObjectPtr<UAssemblySlotComponent>> ChildSlots;

    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    bool IsCompleted() { return IsSelfCompleted() && bIsAllChildrenAssembled; };

    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    virtual void AttachToSlot(UAssemblySlotComponent* Slot);

    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    virtual void DetachFromSlot();

    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    virtual bool CanDetachNode() const;

    UFUNCTION(BlueprintCallable, Category = "AssemblyToolBase")
    virtual void OnClicked() {}

    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    virtual bool IsSelfCompleted() { return bIsRootNode || ParentSlot; };

    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    virtual void UpdateChildrenAssemblyStatus();

    UFUNCTION(BlueprintCallable, Category = "AssemblyNodeBase")
    void SetHighlightEnabled(bool bEnable) 
    { 
        MeshComponent->SetRenderCustomDepth(bEnable);

        // 设置 Stencil 值（可以用不同数字代表不同颜色的外发光，如 1=绿色高亮, 2=黄色预警）
        MeshComponent->SetCustomDepthStencilValue(1);
    };

    virtual void OnConvertFromActor(AActor* Actor) {}

protected:
    /** 根据安装状态自动开启/关闭身上插槽的碰撞触发区 */
    void RefreshChildSlotsActivation();
};