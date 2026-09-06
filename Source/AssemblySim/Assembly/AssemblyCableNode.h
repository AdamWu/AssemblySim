#pragma once

#include "CoreMinimal.h"
#include "AssemblyNodeBase.h"
#include "AssemblyCableNode.generated.h"

class USphereComponent;
class AAssemblyToolBase;
class UWidgetComponent;
class UTimelineComponent;

UCLASS()
class ASSEMBLYSIM_API AAssemblyCableNode : public AAssemblyNodeBase
{
	GENERATED_BODY()

public:
	AAssemblyCableNode();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyCableNode")
	UTimelineComponent* TimelineComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyCableNode")
	USceneComponent* FixedAnchorComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyCableNode")
	UCurveFloat* Curve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyCableNode")
	FTransform TargetOffsetTransform;

public:
	virtual void OnClicked();

	virtual bool IsSelfCompleted() { return bIsClosed; };

	UFUNCTION(BlueprintCallable, Category = "AssemblyCableNode")
	void Play();

private:
	UFUNCTION()
	void HandleTimelineUpdate(float OutputValue);

	UFUNCTION()
	void HandleTimelineFinished();

	FTransform InitPivotTransform; 
	FTransform TargetPivotTransform;
	
};