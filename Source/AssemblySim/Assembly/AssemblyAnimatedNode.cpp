#include "AssemblyAnimatedNode.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TimelineComponent.h"
#include "AssemblySlotComponent.h"
#include "AssemblyToolBase.h"

AAssemblyAnimatedNode::AAssemblyAnimatedNode()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent->SetMobility(EComponentMobility::Movable);

	bIsClosed = true;

	TimelineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("TimelineComponent"));
}

void AAssemblyAnimatedNode::BeginPlay()
{
	Super::BeginPlay();

	if (Curve)
	{
		FOnTimelineFloat UpdateDelegate;
		UpdateDelegate.BindUFunction(this, FName("HandleTimelineUpdate"));
		TimelineComponent->AddInterpFloat(Curve, UpdateDelegate);

		FOnTimelineEvent FinishedDelegate;
		FinishedDelegate.BindUFunction(this, FName("HandleTimelineFinished"));
		TimelineComponent->SetTimelineFinishedFunc(FinishedDelegate);

		TimelineComponent->SetLooping(false);
	}

	InitPivotTransform = RootComponent->GetRelativeTransform();
	TargetPivotTransform = TargetOffsetTransform * InitPivotTransform;

	const auto& Q = InitPivotTransform.GetRotation();
	const FVector InitLocation = InitPivotTransform.GetLocation();
	const FQuat InitRotation = InitPivotTransform.GetRotation();
	const FVector InitScale = InitPivotTransform.GetScale3D();
	//TargetPivotTransform.SetLocation(InitLocation + InitRotation.RotateVector(TargetOffsetTransform.GetLocation()));
	//TargetPivotTransform.SetRotation(InitRotation * TargetOffsetTransform.GetRotation());
	//TargetPivotTransform.SetScale3D(InitScale * TargetOffsetTransform.GetScale3D());

	UE_LOG(LogTemp, Log, TEXT("BeginPlay %s"), *GetName());
}

void AAssemblyAnimatedNode::Play()
{
	if (TimelineComponent->IsPlaying()) return;

	UE_LOG(LogTemp, Log, TEXT("Play %s"), *GetName());

	if (bIsClosed) TimelineComponent->PlayFromStart();
	else TimelineComponent->ReverseFromEnd();

}


void AAssemblyAnimatedNode::OnClicked()
{
	if (ParentSlot && ParentSlot->bIsLocked) return;

	UE_LOG(LogTemp, Log, TEXT("OnClicked %s"), *GetName());

	Play();
}

void AAssemblyAnimatedNode::HandleTimelineUpdate(float OutputValue)
{

	FTransform CurrentTransform;
	CurrentTransform.Blend(InitPivotTransform, TargetPivotTransform, OutputValue);

	RootComponent->SetRelativeTransform(CurrentTransform);
}

void AAssemblyAnimatedNode::HandleTimelineFinished()
{
	bIsClosed = !bIsClosed;

	UE_LOG(LogTemp, Log, TEXT("HandleTimelineFinished %s"), *GetName());

	UpdateChildrenAssemblyStatus();

	// update slot for attached node
	if (ParentSlot) {
		if (bIsClosed) ParentSlot->OccupySlot(this);
		else  ParentSlot->ClearSlot();
	}

}
