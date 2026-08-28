// AssemblyInteractionManager.cpp
#include "AssemblyInteractionManager.h"
#include "Kismet/GameplayStatics.h"
#include "AssemblyNodeBase.h"
#include "AssemblySlotComponent.h"
#include "AssemblyToolBase.h"
#include "AssemblyFastenerNode.h"

AAssemblyInteractionManager::AAssemblyInteractionManager()
{
	PrimaryActorTick.bCanEverTick = true;

	AutoReceiveInput = EAutoReceiveInput::Player0;
}

void AAssemblyInteractionManager::BeginPlay()
{
	Super::BeginPlay();
	PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
	}
}


void AAssemblyInteractionManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!PC || !HeldActor) return;

	AAssemblyNodeBase* HitNode = Cast<AAssemblyNodeBase>(HeldActor);
	AAssemblyToolBase* HitTool = Cast<AAssemblyToolBase>(HeldActor);

	// node cannot move
	if (HitNode && !HitNode->CanDetachNode()) return;

	// check detach for drag distance
	if (HeldActorDetachLocked) {

		FHitResult HitResult;
		if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
		{
			float DragDistance = FVector::Distance(HitResult.Location, DragOrigin);
			UE_LOG(LogTemp, Log, TEXT("DragDistance %f"), DragDistance);
			if (DragDistance > 10)
			{
				HeldActorDetachLocked = false;
				// node
				if (HitNode && HitNode->ParentSlot) {
					HitNode->DetachFromSlot();
				}
				// tool
				if (HitTool && HitTool->AttachedNode) {
					HitTool->DetachFromNode();
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
		QueryParams.AddIgnoredActor(HeldActor);
		FHitResult HitResult;
		bool bHitOther = GetWorld()->LineTraceSingleByChannel(HitResult, WorldLoc, WorldLoc + WorldDir * 10000, ECC_Visibility, QueryParams);

		FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
		FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
		FVector CameraForward = CameraRotation.Vector();
		FVector DynamicPlaneAnchor = CameraLocation + CameraForward * DragZ;
		FVector DynamicPlaneNormal = CameraForward;

		float Denominator = FVector::DotProduct(WorldDir, DynamicPlaneNormal);
		if (!FMath::IsNearlyZero(Denominator))
		{
			float T = FVector::DotProduct(DynamicPlaneAnchor - WorldLoc, DynamicPlaneNormal) / Denominator;
			if (bHitOther)
			{
				FTransform CameraTransform(CameraRotation, CameraLocation);
				FVector CameraLocalHitPoint = CameraTransform.InverseTransformPosition(HitResult.Location);
				UE_LOG(LogTemp, Log, TEXT("T %f %f"), T, CameraLocalHitPoint.X);
				T = FMath::Min(T, CameraLocalHitPoint.X);
			}
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

	FHitResult HitResult;
	AActor* HitActor = nullptr;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
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

	HeldActor = HitActor;

	// 记录抓取时与摄像机的距离
	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
	FTransform CameraTransform(CameraRotation, CameraLocation);
	FVector CameraLocalHitPoint = CameraTransform.InverseTransformPosition(HitResult.Location);
	DragZ = CameraLocalHitPoint.X;
	DragOrigin = HitResult.Location;
	FVector Offset = HeldActor->GetActorLocation() - HitResult.Location;
	DragOffset = CameraRotation.UnrotateVector(Offset);
}

void AAssemblyInteractionManager::OnMouseLeftReleased()
{
	if (!HeldActor) return;

	if (AAssemblyNodeBase* HitNode = Cast<AAssemblyNodeBase>(HeldActor)) {
		UAssemblySlotComponent* TargetSlot = FindOverlappingSlot(HitNode);
		if (TargetSlot && TargetSlot->CanAccept(HitNode))
		{
			HitNode->AttachToSlot(TargetSlot);
		}
		else {
			HitNode->OnClicked();
		}
	}

	// tool
	if (AAssemblyToolBase* HitTool = Cast<AAssemblyToolBase>(HeldActor)) {
		AAssemblyFastenerNode* TargetNode = FindOverlappingFastenerNode(HitTool);
		if (TargetNode && TargetNode->CanAcceptTool(HitTool))
		{
			HitTool->AttachToNode(TargetNode);
		}
		else {
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