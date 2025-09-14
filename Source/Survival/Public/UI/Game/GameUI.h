#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "GameUI.generated.h"

// This class does not need to be modified.
UCLASS(Abstract, Blueprintable)
class SURVIVAL_API UGameUI : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeConstruct() override;

	virtual void BeginDestroy() override;

	UFUNCTION()
	void OnControlledPawnChanged(class APawn* PrevPawn, class APawn* NewPawn);
	UFUNCTION()
	void OnCurrentWeaponChanged(class UEquipmentComponent* Equipment, TSubclassOf<UInventoryWeaponDefinition> NewWeapon);
	UFUNCTION()
	void OnInventoryModified(UInventoryComponent* Inventory, TSubclassOf<UInventoryItemDefinition> Item, int32 PrevCount, int32 NewCount);

	void UpdateCurrentWeapon(TSubclassOf<UInventoryWeaponDefinition> NewWeapon);
	void UpdateCurrentAmmo();

public:
	
	UPROPERTY(meta=(BindWidget))
	class UImage* WeaponImage = nullptr;

	UPROPERTY(meta=(BindWidget)) 
	class UTextBlock* AmmoCountTextBlock = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UTexture2D* NoWeaponTexture = nullptr;

	UPROPERTY()
	TSubclassOf<UInventoryWeaponDefinition> CurrentWeapon;
};
