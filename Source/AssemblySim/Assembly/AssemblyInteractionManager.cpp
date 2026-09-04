// AssemblyInteractionManager.cpp
#include "AssemblyInteractionManager.h"
#include "Kismet/GameplayStatics.h"
#include "AssemblyNodeBase.h"
#include "AssemblySlotComponent.h"
#include "AssemblyToolBase.h"
#include "AssemblyFastenerNode.h"
#include "AssemblyPlayerController.h"
#include "AssemblyGameMode.h"

AAssemblyInteractionManager::AAssemblyInteractionManager()
{
	PrimaryActorTick.bCanEverTick = true;

	AutoReceiveInput = EAutoReceiveInput::Player0;
}

void AAssemblyInteractionManager::BeginPlay()
{
	Super::BeginPlay();
	PC = Cast<AAssemblyPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (PC)
	{
		//PC->bShowMouseCursor = true;
		//PC->bEnableClickEvents = true;
	}
}


void AAssemblyInteractionManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!PC || !HeldActor) return;

	AAssemblyGameMode* GameMode = Cast<AAssemblyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	AAssemblyNodeBase* HitNode = Cast<AAssemblyNodeBase>(HeldActor);
	AAssemblyToolBase* HitTool = Cast<AAssemblyToolBase>(HeldActor);

	// node cannot move
	if (HitNode && !HitNode->CanDetachNode()) return;

	// check detach for drag distance
	if (HeldActorDetachLocked) {

		FHitResult HitResult;
		if (PC->GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
		{
			float DragDistance = FVector::Distance(HitResult.Location, DragOrigin);
			UE_LOG(LogTemp, Log, TEXT("DragDistance %f"), DragDistance);
			if (DragDistance > 10)
			{
				HeldActorDetachLocked = false;
				HeldActorDetachTriggered = true;

				// node
				if (HitNode && HitNode->ParentSlot) 
				{
					if (GameMode->OnCheckStep(EAssemblyAction::Snap, HitNode->NodeID, HitNode->ParentSlot->SlotID))
					{
						HitNode->DetachFromSlot();
					}
					else {
						HitNode->SetActorLocation(DragActorLocation);
						HeldActor = nullptr;
					}

				}
				// tool
				if (HitTool && HitTool->AttachedNode) {
					if (GameMode->OnCheckStep(EAssemblyAction::Snap, HitTool->ToolID, HitTool->AttachedNode->NodeID))
					{
						HitTool->DetachFromNode();
					}
					else {
						HitTool->SetActorLocation(DragActorLocation);
						HeldActor = nullptr;
					}
				}
			}
		}
		return;
	}


	FVector WorldLoc, WorldDir;
	if (PC->DeprojectMousePositionToWorld(WorldLoc, WorldDir))
	{
		//FVector NewLocation = WorldLoc + (WorldDir * DragZ) - DragOffset;
		//HeldActor->SetActorLocation(NewLocation);

		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = true;
		QueryParams.AddIgnoredActor(HeldActor);
		FHitResult HitResult;
		bool bHitOther = GetWorld()->LineTraceSingleByChannel(HitResult, WorldLoc, WorldLoc + WorldDir * DragBaseZ, ECC_Visibility, QueryParams);
		if (bHitOther)
		{
			HeldActor->SetActorLocation(HitResult.Location);
			return;
		}

		FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
		FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
		FVector CameraForward = CameraRotation.Vector();
		FVector DynamicPlaneAnchor = CameraLocation + CameraForward * DragZ;
		FVector DynamicPlaneNormal = CameraForward;

		float Denominator = FVector::DotProduct(WorldDir, DynamicPlaneNormal);
		if (!FMath::IsNearlyZero(Denominator))
		{
			float T = FVector::DotProduct(DynamicPlaneAnchor - WorldLoc, DynamicPlaneNormal) / Denominator;

			FVector CurrentHitLocation = WorldLoc + WorldDir * T;
			FVector CurrentWorldGrabOffset = CameraRotation.RotateVector(DragOffset);
			FVector TargetLocation = CurrentHitLocation + CurrentWorldGrabOffset;
			HeldActor->SetActorLocation(TargetLocation);
		}
	}
}

void AAssemblyInteractionManager::OnMouseLeftPressed()
{
	if (!PC) return;
	PC->SetDraggingState(true);

	FHitResult HitResult;
	AActor* HitActor = nullptr;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		HitActor = HitResult.GetActor();
	}
	if (!HitActor) return;

	AAssemblyNodeBase* HitNode = Cast<AAssemblyNodeBase>(HitActor);
	AAssemblyToolBase* HitTool = Cast<AAssemblyToolBase>(HitActor);

	HeldActorDetachLocked = false;
	if (HitNode && HitNode->ParentSlot || HitTool && HitTool->AttachedNode) {
		HeldActorDetachLocked = true;
	}
	
	HeldActorDetachTriggered = false;

	HeldActor = HitActor;

	// 记录抓取时与摄像机的距离
	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
	FTransform CameraTransform(CameraRotation, CameraLocation);
	FVector CameraLocalActorPoint = CameraTransform.InverseTransformPosition(HeldActor->GetActorLocation());
	FVector CameraLocalHitPoint = CameraTransform.InverseTransformPosition(HitResult.Location);
	DragBaseZ = CameraLocalActorPoint.X;
	DragZ = CameraLocalHitPoint.X;
	DragOrigin = HitResult.Location;
	FVector Offset = HeldActor->GetActorLocation() - HitResult.Location;
	DragOffset = CameraRotation.UnrotateVector(Offset);
	DragActorLocation = HeldActor->GetActorLocation();
}

void AAssemblyInteractionManager::OnMouseLeftReleased()
{
	if (!PC) return;
	PC->SetDraggingState(false);

	if (!HeldActor) return;

	AAssemblyGameMode* GameMode = Cast<AAssemblyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	if (AAssemblyNodeBase* HitNode = Cast<AAssemblyNodeBase>(HeldActor)) {
		UAssemblySlotComponent* TargetSlot = FindOverlappingSlot(HitNode);
		if (TargetSlot && TargetSlot->CanAccept(HitNode))
		{
			if (GameMode->OnCheckStep(EAssemblyAction::Snap, HitNode->NodeID, TargetSlot->SlotID))
				HitNode->AttachToSlot(TargetSlot);
		}
		else {
			if (GameMode->OnCheckStep(EAssemblyAction::Click, HitNode->NodeID, NAME_None))
				HitNode->OnClicked();
		}
	}

	// tool
	if (AAssemblyToolBase* HitTool = Cast<AAssemblyToolBase>(HeldActor)) {
		AAssemblyFastenerNode* TargetNode = FindOverlappingFastenerNode(HitTool);
		if (TargetNode && TargetNode->CanAcceptTool(HitTool))
		{
			if (GameMode->OnCheckStep(EAssemblyAction::Snap, HitTool->ToolID, TargetNode->NodeID))
				HitTool->AttachToNode(TargetNode);
		}
		else if(HitTool->AttachedNode){
			if (GameMode->OnCheckStep(EAssemblyAction::Click, HitTool->ToolID, NAME_None))
				HitTool->OnClicked();
		}
	}

	HeldActor = nullptr;
}

AActor* AAssemblyInteractionManager::GetNodeUnderCursor() const
{
	return nullptr;
}

// 寻找 Node 碰到的 SlotComponent
UAssemblySlotComponent* AAssemblyInteractionManager::FindOverlappingSlot(AAssemblyNodeBase* Node) const
{
	if (!Node || !Node->MeshComponent) return nullptr;

	TArray<UPrimitiveComponent*> OverlappingComps;
	Node->MeshComponent->GetOverlappingComponents(OverlappingComps);

	for (UPrimitiveComponent* Comp : OverlappingComps)
	{
		if (UAssemblySlotComponent* Slot = Cast<UAssemblySlotComponent>(Comp))
		{
			return Slot;
		}
		if (UAssemblySlotComponent* Slot = Cast<UAssemblySlotComponent>(Comp->GetAttachParent()))
		{
			return Slot;
		}
		if (UAssemblySlotComponent* Slot = Cast<UAssemblySlotComponent>(Comp->GetOuter()))
		{
			return Slot;
		}
	}
	return nullptr;
}


AAssemblyFastenerNode* AAssemblyInteractionManager::FindOverlappingFastenerNode(AAssemblyToolBase* Tool) const
{
	TArray<AActor*> OverlappingActors;
	Tool->GetOverlappingActors(OverlappingActors, AAssemblyFastenerNode::StaticClass());

	AAssemblyFastenerNode* BestCandidate = nullptr;
	float MinDistanceSq = FLT_MAX;

	for (AActor* Actor : OverlappingActors)
	{
		if (AAssemblyFastenerNode* Fastener = Cast<AAssemblyFastenerNode>(Actor))
		{
			if (Fastener->CanAcceptTool(Tool))
			{
				float DistSq = FVector::DistSquared(Tool->GetActorLocation(), Fastener->GetActorLocation());
				if (DistSq < MinDistanceSq)
				{
					MinDistanceSq = DistSq;
					BestCandidate = Fastener;
				}
			}
		}
	}

	return BestCandidate;
}