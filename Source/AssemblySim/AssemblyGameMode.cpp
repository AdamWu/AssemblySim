// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssemblyGameMode.h"
#include "AssemblyPlayerController.h"
#include "UObject/ConstructorHelpers.h"

AAssemblyGameMode::AAssemblyGameMode()
{
	// 绑定自定义 PlayerController
	PlayerControllerClass = AAssemblyPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP_CameraController"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
