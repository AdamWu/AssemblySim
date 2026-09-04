#pragma once

#include "CoreMinimal.h"
#include "AssemblyNodeBase.h"
#include "AssemblyFastenerNode.generated.h"

class USphereComponent;
class AAssemblyToolBase;
class UWidgetComponent;
class UTimelineComponent;

UCLASS()
class ASSEMBLYSIM_API AAssemblyFastenerNode : public AAssemblyNodeBase
{
	GENERATED_BODY()

public:
	AAssemblyFastenerNode();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	UShapeComponent* TriggerZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	USceneComponent* SnapAnchorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	UWidgetComponent* WidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	UTimelineComponent* TimelineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	FGameplayTag RequiredToolTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	UCurveFloat* FastenCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	float TotalRotationDegrees = 360;

	// cannot be detached
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	bool bIsCaptive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	bool bIsClosed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AssemblyFastenerNode")
	AAssemblyToolBase* CurrentTool = nullptr;

	float LastTimelineValue = 0;

public:
	virtual void AttachToSlot(UAssemblySlotComponent* Slot) override;

	virtual void DetachFromSlot() override;

	virtual bool CanDetachNode() const;

	virtual bool IsSelfCompleted() { return bIsClosed; };

	virtual void UpdateChildrenAssemblyStatus();


	UFUNCTION(BlueprintCallable, Category = "AssemblyFastenerNode")
	bool CanAcceptTool(AAssemblyToolBase* Tool) const;

	UFUNCTION(BlueprintCallable, Category = "AssemblyFastenerNode")
	void UseTool(AAssemblyToolBase* Tool);

	UFUNCTION(BlueprintCallable, Category = "AssemblyFastenerNode")
	void ReleaseTool();

	UFUNCTION(BlueprintCallable, Category = "AssemblyFastenerNode")
	void Fasten();

	virtual void OnConvertFromActor(AActor* Actor);
private:
	UFUNCTION()
	void OnToolOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
	                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
	                        bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnToolOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
	                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	UFUNCTION()
	void HandleTimelineUpdate(float OutputValue);

	UFUNCTION()
	void HandleTimelineFinished();

	void UpdateProgress(float Progress);
};