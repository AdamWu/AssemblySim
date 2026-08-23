#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "AssemblySlotComponent.generated.h"

class AAssemblyNodeBase;
class UShapeComponent;
class UStaticMeshComponent;

// 动态多播委托：通知蓝图播放音效/特效
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlotHoverChanged, bool, bIsHovered, AAssemblyNodeBase*, Node);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotOccupiedChanged, bool, bIsOccupied);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ASSEMBLYSIM_API UAssemblySlotComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UAssemblySlotComponent();

protected:
    virtual void BeginPlay() override;

public:
    // ==========================================
    // 属性配置
    // ==========================================

    /** 该插槽允许接收的部件类型 Tag */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblySlotComponent")
    FGameplayTag AcceptNodeTag;

    /** 是否被占用 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblySlotComponent")
    bool bIsOccupied = false;

    /** 当前吸附在插槽上的部件指针 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblySlotComponent")
    TObjectPtr<AAssemblyNodeBase> CurrentOccupiedNode = nullptr;

    // ==========================================
    // 子组件 (触发区 + 预览网格)
    // ==========================================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblySlotComponent")
    UShapeComponent* TriggerZone;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblySlotComponent")
    TObjectPtr<UStaticMeshComponent> PreviewMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssemblySlotComponent")
    TObjectPtr<UMaterialInterface> PreviewMaterial;

    // ==========================================
    // 事件委托
    // ==========================================

    UPROPERTY(BlueprintAssignable, Category = "AssemblySlotComponent")
    FOnSlotHoverChanged OnSlotHoverChanged;

    UPROPERTY(BlueprintAssignable, Category = "AssemblySlotComponent")
    FOnSlotOccupiedChanged OnSlotOccupiedChanged;

    // ==========================================
    // 核心接口
    // ==========================================

    /** 检查是否允许吸附指定部件 */
    UFUNCTION(BlueprintCallable, Category = "AssemblySlotComponent")
    bool CanAccept(const AAssemblyNodeBase* Node) const;

    /** 执行吸附动作 */
    UFUNCTION(BlueprintCallable, Category = "AssemblySlotComponent")
    bool OccupySlot(AAssemblyNodeBase* Node, bool bSilent = false);

    /** 执行拆卸解绑动作 */
    UFUNCTION(BlueprintCallable, Category = "AssemblySlotComponent")
    bool ClearSlot();

    /** 显隐 Ghost 预览网格 */
    UFUNCTION(BlueprintCallable, Category = "AssemblySlotComponent")
    void SetSlotHovered(bool bHovered, AAssemblyNodeBase* Node = nullptr);

    /** 开启/关闭插槽的触发区碰撞 (用完即关，拔出再开) */
    UFUNCTION(BlueprintCallable, Category = "AssemblySlotComponent")
    void SetSlotActive(bool bActive);

private:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};