#include "UI/MainMenu/MainMenuHUD.h"

#include "UI/MainMenu/MainMenuUI.h"

void AMainMenuHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto* PlayerController = GetOwningPlayerController())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		if (auto* Widget = CreateWidget<UUserWidget>(GetWorld(), MenuUI))
		{
			Widget->AddToViewport(9999);
			InputMode.SetWidgetToFocus(Widget->TakeWidget());
		}
		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = true;
	}
}

void AMainMenuHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (auto* PlayerController = GetOwningPlayerController()) {
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;
	}
}
