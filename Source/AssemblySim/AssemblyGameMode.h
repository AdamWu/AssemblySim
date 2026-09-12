// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Data/AssemblyTypes.h"
#include "AssemblyGameMode.generated.h"


UCLASS()
class AAssemblyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAssemblyGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyGameMode")
	bool bIsExamMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyGameMode")
	UDataTable* SOPDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AssemblyGameMode")
	UDataTable* EntityDataTable;

};