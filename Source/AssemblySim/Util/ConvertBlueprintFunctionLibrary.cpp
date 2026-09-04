// Fill out your copyright notice in the Description page of Project Settings.

#include "Util/ConvertBlueprintFunctionLibrary.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "Components/StaticMeshComponent.h"
#include "ScopedTransaction.h"
#include "Editor/TransBuffer.h"

TArray<AAssemblyNodeBase*> UConvertBlueprintFunctionLibrary::ConvertActorsToAssemblyClass(TSubclassOf<AAssemblyNodeBase> TargetClass)
{
	TArray<AAssemblyNodeBase*> CreatedNodes;

	if (!GEditor || !TargetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[AssemblyEditor] TargetClass null"));
		return CreatedNodes;
	}

	// 获取选中 Actor
	USelection* SelectedActors = GEditor->GetSelectedActors();
	TArray<AActor*> ActorsToReplace;
	for (FSelectionIterator It(*SelectedActors); It; ++It)
	{
		if (AActor* Actor = Cast<AActor>(*It))
		{
			ActorsToReplace.Add(Actor);
		}
	}

	if (ActorsToReplace.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AssemblyEditor] no Actor！"));
		return CreatedNodes;
	}

	// 替换Actor
	UWorld* World = GEditor->GetEditorWorldContext().World();
	for (AActor* OldActor : ActorsToReplace)
	{
		// 尝试获取旧 Actor 的 StaticMeshComponent
		UStaticMeshComponent* OldSMC = OldActor->FindComponentByClass<UStaticMeshComponent>();

		// 暂存基础变换与信息
		FTransform OldTransform = OldActor->GetActorTransform();
		FString OldLabel = OldActor->GetActorLabel();
		FName OldFolderPath = OldActor->GetFolderPath();
		AActor* OldAttachParent = OldActor->GetAttachParentActor();
		FName OldAttachSocket = OldActor->GetAttachParentSocketName();

		UStaticMesh* MeshAsset = OldSMC ? OldSMC->GetStaticMesh() : nullptr;
		TArray<UMaterialInterface*> OverrideMaterials = OldSMC ? OldSMC->OverrideMaterials : TArray<UMaterialInterface*>();

		// 实例化新的 AAssemblyNodeBase 派生类
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = OldActor->GetOwner();

		AAssemblyNodeBase* NewNode = World->SpawnActor<AAssemblyNodeBase>(TargetClass, OldTransform, SpawnParams);
		if (!NewNode) continue;

		NewNode->Modify();
		OldActor->Modify();

		// 基础属性
		NewNode->SetActorLabel(OldLabel);
		NewNode->SetFolderPath(OldFolderPath);

		// 还原 StaticMesh 与 Override Materials
		UStaticMeshComponent* NewSMC = NewNode->FindComponentByClass<UStaticMeshComponent>();
		if (NewSMC && MeshAsset)
		{
			NewSMC->Modify();
			NewSMC->SetStaticMesh(MeshAsset);
			for (int32 i = 0; i < OverrideMaterials.Num(); ++i)
			{
				if (OverrideMaterials[i])
				{
					NewSMC->SetMaterial(i, OverrideMaterials[i]);
				}
			}
		}

		// 恢复父子挂载关系与重定向子级
		if (OldAttachParent)
		{
			NewNode->AttachToActor(OldAttachParent, FAttachmentTransformRules::KeepWorldTransform, OldAttachSocket);
		}

		TArray<AActor*> AttachedChildren;
		OldActor->GetAttachedActors(AttachedChildren);
		for (AActor* Child : AttachedChildren)
		{
			Child->Modify();
			Child->AttachToActor(NewNode, FAttachmentTransformRules::KeepWorldTransform);
		}

		NewNode->OnConvertFromActor(OldActor);

		// 销毁旧 Actor 并收集新节点
		World->DestroyActor(OldActor);
		CreatedNodes.Add(NewNode);
	}

	// 重新选中生成的 Node
	GEditor->SelectNone(true, true);
	for (AAssemblyNodeBase* Node : CreatedNodes)
	{
		GEditor->SelectActor(Node, true, true);
	}

	UE_LOG(LogTemp, Log, TEXT("[AssemblyEditor] Convert %d Actors"), CreatedNodes.Num());
	return CreatedNodes;
}