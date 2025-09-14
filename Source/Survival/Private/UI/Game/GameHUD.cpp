#include "UI\Game\GameHUD.h"

void AGameHUD::BeginPlay()
{
	if (APlayerController* PlayerController = GetOwningPlayerController())
	{
		if (auto* Widget = CreateWidget<UUserWidget>(GetWorld(), GameUI))
		{
			Widget->AddToViewport();
		}
	}
}

void AGameHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

}
