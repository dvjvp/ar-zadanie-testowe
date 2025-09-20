#pragma once

#include "CoreMinimal.h"
#include "InventoryItemDefinition.generated.h"


UCLASS(Blueprintable, Const, Abstract)
class SURVIVAL_API UInventoryItemDefinition : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 MaxNumInInventory = 1;
};

UCLASS(Blueprintable, Const, Abstract)
class SURVIVAL_API UInventoryWeaponDefinition : public UInventoryItemDefinition
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	class USkeletalMesh* WeaponMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	TSubclassOf<UInventoryItemDefinition> AmmoType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	class UTexture2D* UiIcon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<class UPlayerAbility>> GrantedAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxShootingDistance = 10000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Damage = 20.0f;

	// Number of bullets to add when this weapon is picked up
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	int32 InitialAmmo = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	int32 AmmoGivenOnPickup = 5;
};