#include "Core/Abilities/ShootingAbility.h"
#include "Inventory/InventoryItemDefinition.h"
#include "Inventory/InventoryComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"


UInventoryWeaponDefinition* UShootingAbility::GetOwningWeapon() const
{
	if (GrantedByItem.Get()->IsChildOf(UInventoryWeaponDefinition::StaticClass()))
	{
		return Cast<UInventoryWeaponDefinition>(GrantedByItem.GetDefaultObject());
	}
	return nullptr;
}

int32 UShootingAbility::GetAmmoCount() const
{
	UInventoryWeaponDefinition* ThisWeapon = GetOwningWeapon();
	UInventoryComponent* Inventory = GetOwner()->GetComponentByClass<UInventoryComponent>();
	if (ThisWeapon && Inventory)
	{
		return Inventory->GetItemCount(ThisWeapon->AmmoType);
	}

	return 0;
}

int32 UShootingAbility::UseAmmo(int32 NumToUse)
{
	UInventoryWeaponDefinition* ThisWeapon = GetOwningWeapon();
	UInventoryComponent* Inventory = GetOwner()->GetComponentByClass<UInventoryComponent>();
	if (ThisWeapon && Inventory)
	{
		return Inventory->RemoveItems(ThisWeapon->AmmoType, NumToUse);
	}

	return 0;
}

void UShootingAbility::FireWeapon(FVector ShotOrigin, FVector ShotDirection)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, FString::Printf(TEXT("[Client] FireWeapon (Client: %s)"), *GetOwner()->GetName()));
	
	Rpc_RequestWeaponFire(GetOwner()->GetActorLocation(), ShotOrigin, ShotDirection);
}

void UShootingAbility::Rpc_RequestWeaponFire_Implementation(FVector ClientActorLocation, FVector ShotOrigin, FVector ShotDirection)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("[Server] RequestWeaponFire"));

	// If data sent from the client is within this margin of what we got simulated
	// on the server, we treat it as valid
	const float DistanceTreshold = FMath::Square(100.0f);

	if (GetAmmoCount() < 0)
	{
		return;
	}

	const FVector ServerActorLocation = GetOwner()->GetActorLocation();
	if (FVector::DistSquared(ServerActorLocation, ClientActorLocation) > DistanceTreshold)
	{
		return;
	}

	if (FVector::DistSquared(ServerActorLocation, ShotOrigin) > DistanceTreshold)
	{
		return;
	}

	// There should be more checks, i.e. for if the delay between consecutive shots requested by client is lower
	// than what the ability allows

	UInventoryWeaponDefinition* Weapon = GetOwningWeapon();
	if (!Weapon)
	{
		return;
	}

	// Data seems to be valid, let's perform the shot
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, 
		ShotOrigin, ShotOrigin + (ShotDirection * Weapon->MaxShootingDistance),
		ECollisionChannel::ECC_GameTraceChannel1, Params))
	{
// 		if (HitResult.GetActor() && HitResult.GetActor()->CanBeDamaged())
// 		{
// 			HitResult.GetActor()->TakeDamage(Weapon->Damage, )
// 		}

		Rpc_ShowEffectsOnWeaponFired(HitResult.ImpactPoint, HitResult.GetActor());
	}

	if (!IsLocalCopy()) // Local copy already uses ammo on its own - this needs to be changed!
	{
		UseAmmo(1);
	}
}

void UShootingAbility::Rpc_ShowEffectsOnWeaponFired_Implementation(FVector ShotImpactPoint, AActor* HitActor)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("[Multicast] Pew pew pew"));

	const bool HitSomeone = Cast<ACharacter>(HitActor) != nullptr;

	DrawDebugLine(GetWorld(), GetMuzzleLocation(), ShotImpactPoint, HitSomeone ? FColor::Red : FColor::Green, false, 3.0f, 0, 3.0f);
}
