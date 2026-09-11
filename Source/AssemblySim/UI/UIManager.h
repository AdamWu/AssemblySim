// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UIManager.generated.h"

USTRUCT(BlueprintType)
struct FWindowInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WindowInfo")
	TSubclassOf<UUserWidget> Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WindowInfo")
	bool bFullScreen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WindowInfo")
	bool bHideLastWindow;
};

USTRUCT(BlueprintType)
struct FUIWindow
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIWindow")
	UUserWidget* UserWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIWindow")
	FWindowInfo WindowInfo;

	FUIWindow() = default;
	FUIWindow(UUserWidget* UserWidget, FWindowInfo& WindowInfo) :UserWidget(UserWidget), WindowInfo(WindowInfo){}
};

UCLASS()
class ASSEMBLYSIM_API AUIManager : public AActor
{
	GENERATED_BODY()
	
public:
	AUIManager();

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	static AUIManager* GetInstance() { return Instance; }

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	static UUserWidget* OpenWindowByName(const FString panelName)
	{
		return AUIManager::GetInstance()->OpenWindowByNameInternal(panelName);
	}

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	static void CloseWindow(UUserWidget* Widget)
	{
		AUIManager::GetInstance()->CloseWindowInternal(Widget);
	}

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	static void CloseAllWindows()
	{
		AUIManager::GetInstance()->CloseAllWindowsInternal();
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UUserWidget* OpenWindowByNameInternal(const FString& WindowName);

	void CloseWindowInternal(UUserWidget* Widget);

	void CloseAllWindowsInternal();

public:
	UPROPERTY(EditAnywhere, Category = "UIManager")
	TMap<FString, FWindowInfo> WindowInfos;

private:
	TArray<FUIWindow> WindowStack;

	static AUIManager* Instance;
};