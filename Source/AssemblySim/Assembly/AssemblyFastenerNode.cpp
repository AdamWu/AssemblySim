#include "AssemblyFastenerNode.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "AssemblySlotComponent.h"
#include "AssemblyToolBase.h"

AAssemblyFastenerNode::AAssemblyFastenerNode()
{
	PrimaryActorTick.bCanEverTick = false;

	SnapAnchorComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SnapAnchor"));
	SnapAnchorComponent->SetupAttachment(RootComponent);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetVisibility(false);

	TriggerZone = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	TriggerZone->SetupAttachment(RootComponent);
	TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
	//TriggerZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TimelineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("TimelineComponent"));
}

void AAssemblyFastenerNode::BeginPlay()
{
	Super::BeginPlay();

	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &AAssemblyFastenerNode::OnToolOverlapBegin);
	TriggerZone->OnComponentEndOverlap.AddDynamic(this, &AAssemblyFastenerNode::OnToolOverlapEnd);

	if (FastenCurve)
	{
		FOnTimelineFloat UpdateDelegate;
		UpdateDelegate.BindUFunction(this, FName("HandleTimelineUpdate"));
		TimelineComponent->AddInterpFloat(FastenCurve, UpdateDelegate);

		FOnTimelineEvent FinishedDelegate;
		FinishedDelegate.BindUFunction(this, FName("HandleTimelineFinished"));
		TimelineComponent->SetTimelineFinishedFunc(FinishedDelegate);

		TimelineComponent->SetLooping(false);
		TimelineComponent->SetPlayRate(1.0f / CurveDuration);
	}

	InitQuat = RootComponent->GetRelativeRotation().Quaternion();
}


void AAssemblyFastenerNode::AttachToSlot(UAssemblySlotComponent* Slot)
{
	Super::AttachToSlot(Slot);

	TriggerZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}


void AAssemblyFastenerNode::DetachFromSlot()
{
	Super::DetachFromSlot();

	TriggerZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool AAssemblyFastenerNode::CanDetachNode() const
{
	bool ret = Super::CanDetachNode();
	return ret && !bIsClosed && !CurrentTool && !bIsCaptive;
}


void AAssemblyFastenerNode::UpdateChildrenAssemblyStatus()
{
	// refresh children status
	Super::UpdateChildrenAssemblyStatus();

	// lock children when fastened
	for (UAssemblySlotComponent* Slot : ChildSlots)
	{
		if (Slot) Slot->bIsLocked = bIsClosed;
	}
}

bool AAssemblyFastenerNode::CanAcceptTool(AAssemblyToolBase* Tool) const
{
	if (CurrentTool || !Tool) return false;

	return Tool->ToolTag.MatchesTagExact(RequiredToolTag);
}

void AAssemblyFastenerNode::UseTool(AAssemblyToolBase* Tool)
{
	CurrentTool = Tool;
	SetHighlightEnabled(true);

	UpdateProgress(bIsClosed ? 1 : 0);
	WidgetComponent->SetVisibility(true);
}

void AAssemblyFastenerNode::ReleaseTool()
{
	CurrentTool = nullptr;
	SetHighlightEnabled(false);

	WidgetComponent->SetVisibility(false);
}


void AAssemblyFastenerNode::Fasten()
{
	if (TimelineComponent->IsPlaying()) return;

	UE_LOG(LogTemp, Log, TEXT("Fasten %s"), *GetName());

	CurrentCurveIdx = 0;
	TimelineComponent->PlayFromStart();
}

void AAssemblyFastenerNode::OnToolOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                                bool bFromSweep, const FHitResult& SweepResult)
{
	AAssemblyToolBase* Tool = Cast<AAssemblyToolBase>(OtherActor);
	if (Tool && CanAcceptTool(Tool))
	{
		SetHighlightEnabled(true);
	}
}

void AAssemblyFastenerNode::OnToolOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetHighlightEnabled(false);
}


void AAssemblyFastenerNode::HandleTimelineUpdate(float OutputValue)
{
	FTransform SnapToRoot = SnapAnchorComponent->GetRelativeTransform();	
	FVector AxisInRootSpace = SnapToRoot.TransformVectorNoScale(FVector::UpVector).GetSafeNormal();

	float DeltaAngle = OutputValue * CurveDegrees * (bIsClosed ? -1 : 1);
	FQuat DeltaQuat = FQuat(AxisInRootSpace, FMath::DegreesToRadians(DeltaAngle));

	FQuat CurrentQuat = RootComponent->GetRelativeRotation().Quaternion();
	FVector CurrentLocation = RootComponent->GetRelativeLocation();

	FQuat Quat = DeltaQuat * InitQuat;
	RootComponent->SetRelativeRotation(Quat);
}

void AAssemblyFastenerNode::HandleTimelineFinished()
{
	CurrentCurveIdx++;

	UE_LOG(LogTemp, Log, TEXT("HandleTimelineFinished %s"), *GetName());

	float Progress = CurrentCurveIdx / (float)CurveTimes;

	if (CurrentCurveIdx < CurveTimes)
	{
		TimelineComponent->PlayFromStart();
		UpdateProgress(bIsClosed ? 1 - Progress : Progress);
	}
	else
	{
		bIsClosed = !bIsClosed;
		UpdateProgress(bIsClosed ? Progress : 1 - Progress);

		// refresh children status
		UpdateChildrenAssemblyStatus();
	}


}


void AAssemblyFastenerNode::UpdateProgress(float Progress)
{
	UUserWidget* UserWidget = WidgetComponent->GetWidget();
	if (UserWidget) {
		FString Str = FString::Printf(TEXT("%d%%"), (int)(Progress * 100));
		UTextBlock* Label = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextBlock")));
		if (Label) Label->SetText(FText::FromString(Str));
		UProgressBar* ProgressBar = Cast<UProgressBar>(UserWidget->GetWidgetFromName(TEXT("ProgressBar")));
		if (ProgressBar) ProgressBar->SetPercent(Progress);
	}
}


void AAssemblyFastenerNode::OnConvertFromActor(AActor* Actor)
{
	UStaticMesh* Mesh = MeshComponent->GetStaticMesh();
	if (!Mesh) return;

	const FBoxSphereBounds& Bounds = Mesh->GetBounds();

	if (USphereComponent* SphereComp = Cast<USphereComponent>(TriggerZone)) {

		SphereComp->SetRelativeLocation(Bounds.Origin);
		SphereComp->SetSphereRadius(Bounds.SphereRadius);
	}

	
}