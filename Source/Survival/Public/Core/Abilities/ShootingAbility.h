#pragma once

#include "CoreMinimal.h"
#include "PlayerAbility.h"
#include "ShootingAbility.generated.h"


UCLASS(Blueprintable)
class SURVIVAL_API UShootingAbility : public UPlayerAbility
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintPure)
	FVector GetMuzzleLocation() const;

	UFUNCTION(BlueprintPure)
	UInventoryWeaponDefinition* GetOwningWeapon() const;

	UFUNCTION(BlueprintPure)
	int32 GetAmmoCount() const;

	// Returns number actually used
	UFUNCTION(BlueprintCallable)
	int32 UseAmmo(int32 NumToUse = 1);

	UFUNCTION(BlueprintCallable)
	void FireWeapon(FVector ShotOrigin, FVector ShotDirection);

protected:

	UFUNCTION(Server, Reliable)
	void Rpc_RequestWeaponFire(FVector ActorLocation, FVector ShotOrigin, FVector ShotDirection);

	UFUNCTION(NetMulticast, Unreliable)
	void Rpc_ShowEffectsOnWeaponFired(FVector ShotImpactPoint, AActor* HitActor);
};
