// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Assembly/AssemblyNodeBase.h"
#include "ConvertBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ASSEMBLYSIM_API UConvertBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Assembly|Editor")
	static TArray<AAssemblyNodeBase*> ConvertActorsToAssemblyClass(TSubclassOf<AAssemblyNodeBase> TargetClass);
};
