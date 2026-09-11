// Fill out your copyright notice in the Description page of Project Settings.


#include "UIManager.h"
#include "Blueprint/UserWidget.h"

AUIManager* AUIManager::Instance = nullptr;

AUIManager::AUIManager()
{
	Instance = this;
}

// Called when the game starts or when spawned
void AUIManager::BeginPlay()
{
	Super::BeginPlay();	
}

UUserWidget* AUIManager::OpenWindowByNameInternal(const FString& WindowName)
{
	if (!WindowInfos.Contains(WindowName))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot find UI, WindowName:%s"), *WindowName);
		return nullptr;
	}

	// update pre-windows
	FWindowInfo& Info = WindowInfos[WindowName];
	if (Info.bFullScreen)
	{
		for (int i = WindowStack.Num() - 1; i >= 0; --i)
		{
			FUIWindow last = WindowStack[i];
			last.UserWidget->SetVisibility(ESlateVisibility::Hidden);
			last.UserWidget->SetIsEnabled(false);
			if (last.WindowInfo.bFullScreen) break;
		}
	}
	else if (Info.bHideLastWindow && WindowStack.Num() > 0)
	{
		FUIWindow last = WindowStack[WindowStack.Num() - 1];
		last.UserWidget->SetVisibility(ESlateVisibility::Hidden);
		last.UserWidget->SetIsEnabled(false);
	}

	// add window
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), Info.Class);
	Widget->AddToViewport();
	WindowStack.Push(FUIWindow(Widget, Info));
	return Widget;
}

void AUIManager::CloseWindowInternal(UUserWidget* Widget)
{
	int WindowIdx = 0;
	for (; WindowIdx < WindowStack.Num(); ++WindowIdx)
	{
		if (WindowStack[WindowIdx].UserWidget == Widget) {
			break;
		}
	}
	if (WindowIdx == WindowStack.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("CloseWindowInternal Widget not Found!"));
		return;
	}

	// remove window
	FUIWindow& UIWindow = WindowStack[WindowIdx];
	WindowStack.RemoveAt(WindowIdx);
	Widget->RemoveFromParent();

	// update pre-windows
	FWindowInfo& Info = UIWindow.WindowInfo;
	if (WindowIdx > 0)
	{
		if (Info.bFullScreen)
		{
			bool LastHide = false;
			for (int i = WindowIdx - 1; i >= 0; --i)
			{
				FUIWindow& last = WindowStack[i];
				if (!LastHide)
				{
					last.UserWidget->SetVisibility(ESlateVisibility::Visible);
					last.UserWidget->SetIsEnabled(true);
				}
				LastHide = last.WindowInfo.bHideLastWindow;

				if (last.WindowInfo.bFullScreen) break;
			}
		}
		else if (Info.bHideLastWindow)
		{
			FUIWindow& last = WindowStack[WindowIdx - 1];
			last.UserWidget->SetVisibility(ESlateVisibility::Visible);
			last.UserWidget->SetIsEnabled(true);
		}
	}
}

void AUIManager::CloseAllWindowsInternal()
{
	while (WindowStack.Num() > 0)
	{
		FUIWindow Window = WindowStack.Pop();
		Window.UserWidget->RemoveFromParent();
	}
}

