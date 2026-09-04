#include "AssemblyCableNode.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TimelineComponent.h"
#include "AssemblySlotComponent.h"
#include "AssemblyToolBase.h"

AAssemblyCableNode::AAssemblyCableNode()
{
	PrimaryActorTick.bCanEverTick = false;

	FixedAnchorComponent = CreateDefaultSubobject<USceneComponent>(TEXT("FixedAnchor"));
	FixedAnchorComponent->SetupAttachment(RootComponent);

	RootComponent->SetMobility(EComponentMobility::Movable);

	TimelineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("TimelineComponent"));
}

void AAssemblyCableNode::BeginPlay()
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

	FTransform AnchorWorldTransform = FixedAnchorComponent->GetComponentTransform();
	FTransform NewAnchorWorldTransform = TargetOffsetTransform * AnchorWorldTransform;

	FTransform AnchorLocalTransform = FixedAnchorComponent->GetRelativeTransform();
	TargetPivotTransform = AnchorLocalTransform.Inverse() * NewAnchorWorldTransform;

	UE_LOG(LogTemp, Log, TEXT("BeginPlay %s"), *GetName());
}

void AAssemblyCableNode::Play()
{
	if (TimelineComponent->IsPlaying()) return;

	UE_LOG(LogTemp, Log, TEXT("Play %s"), *GetName());

	if (bIsClosed) TimelineComponent->PlayFromStart();
	else TimelineComponent->ReverseFromEnd();

}


void AAssemblyCableNode::OnClicked()
{
	if (ParentSlot && ParentSlot->bIsLocked) return;

	UE_LOG(LogTemp, Log, TEXT("OnClicked %s"), *GetName());

	Play();
}

void AAssemblyCableNode::HandleTimelineUpdate(float OutputValue)
{

	FTransform CurrentTransform;
	CurrentTransform.Blend(InitPivotTransform, TargetPivotTransform, OutputValue);

	RootComponent->SetRelativeTransform(CurrentTransform);
}

void AAssemblyCableNode::HandleTimelineFinished()
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
