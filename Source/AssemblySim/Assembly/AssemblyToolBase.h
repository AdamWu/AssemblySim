// AssemblyToolBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AssemblyToolBase.generated.h"

class AAssemblyNodeBase;
class USphereComponent;
class UStaticMeshComponent;


UCLASS(Abstract, Blueprintable, ClassGroup = (AssemblySystem))
class ASSEMBLYSIM_API AAssemblyToolBase : public AActor
{
    GENERATED_BODY()

public:
    AAssemblyToolBase();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyToolBase")
    TObjectPtr<USceneComponent> DefaultRoot;

    // 工具的视觉网格体（在蓝图中指定具体模型）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyToolBase")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyToolBase")
    USceneComponent* AnchorComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyToolBase")
    FGameplayTag ToolTag;

    // 当前吸附的零件（nullptr 表示未吸附）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyToolBase")
    AAssemblyFastenerNode* AttachedNode = nullptr;

public:
    UFUNCTION(BlueprintPure, Category = "AssemblyToolBase")
    AAssemblyFastenerNode* GetAttachedNode() const { return AttachedNode; }

    UFUNCTION(BlueprintCallable, Category = "AssemblyToolBase")
    bool AttachToNode(AAssemblyFastenerNode* Node);

    UFUNCTION(BlueprintCallable, Category = "AssemblyToolBase")
    bool DetachFromNode();
};