#pragma once

#include "CoreMinimal.h"
#include "AssemblyNodeBase.h"
#include "AssemblyAnimatedNode.generated.h"

class USphereComponent;
class AAssemblyToolBase;
class UWidgetComponent;
class UTimelineComponent;

UCLASS()
class ASSEMBLYSIM_API AAssemblyAnimatedNode : public AAssemblyNodeBase
{
	GENERATED_BODY()

public:
	AAssemblyAnimatedNode();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyAnimatedNode")
	UTimelineComponent* TimelineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyAnimatedNode")
	UCurveFloat* Curve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyAnimatedNode")
	FTransform TargetOffsetTransform;

public:
	virtual void OnClicked();

	virtual bool IsSelfCompleted() { return bIsClosed; };

	UFUNCTION(BlueprintCallable, Category = "AssemblyAnimatedNode")
	void Play();

private:
	UFUNCTION()
	void HandleTimelineUpdate(float OutputValue);

	UFUNCTION()
	void HandleTimelineFinished();

	FTransform InitPivotTransform; 
	FTransform TargetPivotTransform;
	
};