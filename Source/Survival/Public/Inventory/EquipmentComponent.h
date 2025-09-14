#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentWeaponChanged, UEquipmentComponent*, Equipment, TSubclassOf<UInventoryWeaponDefinition>, NewWeapon);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVAL_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure)
	TSubclassOf<UInventoryWeaponDefinition> GetCurrentWeapon() const;

	UFUNCTION(BlueprintCallable)
	void SwapWeapons(bool DirectionToggle);

	UFUNCTION()
	void OnRep_CurrentWeapon(const TSubclassOf<UInventoryWeaponDefinition>& PreviousValue);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	void SetCurrentWeapon(TSubclassOf<UInventoryWeaponDefinition> NewWeapon);

	UFUNCTION()
	void OnInventoryChanged(class UInventoryComponent* Inventory, TSubclassOf<UInventoryItemDefinition> Item, int32 PrevCount, int32 NewCount);

	UFUNCTION(Server, Reliable)
	void Rpc_ChangeWeaponRemote(TSubclassOf<UInventoryWeaponDefinition> NewWeapon);

	void ChangeWeaponLocally(TSubclassOf<UInventoryWeaponDefinition> NewWeapon);

public:
	UPROPERTY(BlueprintAssignable)
	FOnCurrentWeaponChanged OnCurrentWeaponChanged;

protected:
	UPROPERTY(ReplicatedUsing = "OnRep_CurrentWeapon")
	TSubclassOf<UInventoryWeaponDefinition> CurrentWeapon;

	UPROPERTY()
	TArray<class UPlayerAbility*> WeaponGrantedAbilities;
};
