#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UObject/Object.h"
#include "MainMenuHUD.generated.h"

class UMainMenuUI;

UCLASS(Abstract)
class SURVIVAL_API AMainMenuHUD : public AHUD
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly) TSubclassOf<UMainMenuUI> MenuUI;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
