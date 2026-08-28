#include "AssemblyAnimatedNode.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TimelineComponent.h"
#include "AssemblySlotComponent.h"
#include "AssemblyToolBase.h"

AAssemblyAnimatedNode::AAssemblyAnimatedNode()
{
	PrimaryActorTick.bCanEverTick = false;

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
	TargetPivotTransform = InitPivotTransform * TargetOffsetTransform;
}

void AAssemblyAnimatedNode::Play()
{
	if (TimelineComponent->IsPlaying()) return;

	UE_LOG(LogTemp, Log, TEXT("Play %s"), *GetName());

	if (bIsFinished) TimelineComponent->ReverseFromEnd();
	else TimelineComponent->PlayFromStart();

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
	bIsFinished = !bIsFinished;

	UE_LOG(LogTemp, Log, TEXT("HandleTimelineFinished %s"), *GetName());

	UpdateChildrenAssemblyStatus();

	// update slot for attached node
	if (ParentSlot) {
		if (bIsFinished) ParentSlot->OccupySlot(this);
		else  ParentSlot->ClearSlot();
	}

}