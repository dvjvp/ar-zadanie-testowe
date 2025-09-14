#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameUI.h"
#include "GameHUD.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Abstract)
class SURVIVAL_API AGameHUD : public AHUD
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameUI> GameUI;
	

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
};
