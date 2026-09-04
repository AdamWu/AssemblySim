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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyToolBase")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyToolBase")
    USceneComponent* SnapAnchorComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyToolBase")
    FName ToolID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyToolBase")
    FGameplayTag ToolTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyToolBase")
    AAssemblyFastenerNode* AttachedNode = nullptr;

public:
    UFUNCTION(BlueprintCallable, Category = "AssemblyToolBase")
    void AttachToNode(AAssemblyFastenerNode* Node);

    UFUNCTION(BlueprintCallable, Category = "AssemblyToolBase")
    void DetachFromNode();

    UFUNCTION(BlueprintCallable, Category = "AssemblyToolBase")
    void OnClicked();
};