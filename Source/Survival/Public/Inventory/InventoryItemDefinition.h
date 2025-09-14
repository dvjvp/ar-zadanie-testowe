#pragma once

#include "CoreMinimal.h"
#include "InventoryItemDefinition.generated.h"


UCLASS(Blueprintable, Const, Abstract)
class SURVIVAL_API UInventoryItemDefinition : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxNumInInventory = 1;
};

UCLASS(Blueprintable, Const, Abstract)
class SURVIVAL_API UInventoryWeaponDefinition : public UInventoryItemDefinition
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class USkeletalMesh* WeaponMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UInventoryItemDefinition> AmmoType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UTexture2D* UiIcon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<class UPlayerAbility>> GrantedAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxShootingDistance = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Damage = 20.0f;

	// Number of bullets to add when this weapon is picked up
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 InitialAmmo = 5;
};