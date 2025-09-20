#include "Inventory/EquipmentComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/InventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"
#include "Core/Abilities/PlayerAbility.h"
#include "Core/Player/SurvivalCharacter.h"

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME_CONDITION(UEquipmentComponent, CurrentWeapon, COND_SimulatedOnly);
}

TSubclassOf<UInventoryWeaponDefinition> UEquipmentComponent::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

void UEquipmentComponent::SwapWeapons(bool DirectionToggle)
{
	if (UInventoryComponent* Inventory = GetOwner()->GetComponentByClass<UInventoryComponent>())
	{
		const TArray<TSubclassOf<UInventoryWeaponDefinition>>& AllOwnedWeaponTypes = Inventory->GetAllItemsOfType<UInventoryWeaponDefinition>();

		if (CurrentWeapon == nullptr && !AllOwnedWeaponTypes.IsEmpty())
		{
			SetCurrentWeapon(DirectionToggle ? AllOwnedWeaponTypes[0] : AllOwnedWeaponTypes[AllOwnedWeaponTypes.Num() - 1]);
		}
		else if (AllOwnedWeaponTypes.Num() > 1)
		{
			int32 CurrentWeaponIndex = AllOwnedWeaponTypes.IndexOfByKey(CurrentWeapon);
			CurrentWeaponIndex += DirectionToggle ? 1 : AllOwnedWeaponTypes.Num() - 1;
			CurrentWeaponIndex = CurrentWeaponIndex % AllOwnedWeaponTypes.Num();

			SetCurrentWeapon(AllOwnedWeaponTypes[CurrentWeaponIndex]);
		}
	}
}

void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* OwnerAsPawn = Cast<APawn>(GetOwner());

	if (GetOwner()->HasAuthority() || (OwnerAsPawn && OwnerAsPawn->IsLocallyControlled()))
	{
		if (UInventoryComponent* Inventory = GetOwner()->GetComponentByClass<UInventoryComponent>())
		{
			Inventory->OnInventoryChanged.AddDynamic(this, &UEquipmentComponent::OnInventoryChanged);
		}
	}
}

void UEquipmentComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ChangeWeaponLocally(nullptr);
}

void UEquipmentComponent::SetCurrentWeapon(TSubclassOf<UInventoryWeaponDefinition> NewWeapon)
{
	ChangeWeaponLocally(NewWeapon);
	Rpc_ChangeWeaponRemote(CurrentWeapon);
}

void UEquipmentComponent::OnRep_CurrentWeapon(const TSubclassOf<UInventoryWeaponDefinition>& PreviousValue)
{
	if (PreviousValue != CurrentWeapon)
	{
		ChangeWeaponLocally(CurrentWeapon);
	}
}

void UEquipmentComponent::OnInventoryChanged(class UInventoryComponent* Inventory, TSubclassOf<UInventoryItemDefinition> Item, int32 PrevCount, int32 NewCount)
{
	UInventoryItemDefinition* InventoryItemDef = Item.GetDefaultObject();
	if (UInventoryWeaponDefinition* AsWeapon = Cast<UInventoryWeaponDefinition>(InventoryItemDef))
	{
		if (CurrentWeapon == nullptr)
		{
			SetCurrentWeapon(AsWeapon->GetClass());
		}
		else if (CurrentWeapon == AsWeapon->GetClass() && NewCount == 0)
		{
			//#TODO: Handle what happens when the current weapon is removed.
			// Probably switch to the next one or to no weapon at all
			SetCurrentWeapon(nullptr);
		}
	}
}

void UEquipmentComponent::Rpc_ChangeWeaponRemote_Implementation(TSubclassOf<UInventoryWeaponDefinition> NewWeapon)
{
	// Called on the server from the client. Ensure that the client does indeed own this weapon
	if (NewWeapon != nullptr)
	{
		UInventoryComponent* Inventory = GetOwner()->GetComponentByClass<UInventoryComponent>();
		if (!Inventory)
		{
			return;
		}
		if (!Inventory->HasItem(NewWeapon))
		{
			return;
		}
	}

	ChangeWeaponLocally(NewWeapon);
}

void UEquipmentComponent::ChangeWeaponLocally(TSubclassOf<UInventoryWeaponDefinition> NewWeapon)
{
	// When one of the clients is also a host, they will get ChangeWeaponLocally() message twice,
	// first from local simulation and second from the server rpc a client sends to the server.
	// The additional call won't actually do much harm, but it will make BeginPlay and EndPlay
	// trigger an additional time on the ability components, as one component will be created
	// and then almost immediately destroyed right after.
	// Again, shouldn't be an issue, but we don't need to do that at all, so why waste resources
	if (CurrentWeapon == NewWeapon)
	{
		return;
	}

	CurrentWeapon = NewWeapon;
	OnCurrentWeaponChanged.Broadcast(this, CurrentWeapon);

	// Remove all previously granted abilities
	for (int32 i = WeaponGrantedAbilities.Num() - 1; i >= 0; i--)
	{
		// There is currently a bug in Unreal Engine's Enhanced Input system.
		// The bug is that when a component is unregistered, its bound inputs are not being unregistered.
		// Usually that's not a huge issue (except for a small memory leak and slowing down input processing as the game goes on),
		// because if we're creating and deleting components dynamically
		// the UObjectPtr won't match the new one and the leaked binding will point to something invalid
		// and unreal's pointers will treat as nullptr instead and not execute the binding.
		// 
		// But since for networking I made the components have consistent names, even after being deleted
		// and recreated, the UObjectPtrs from old bindings are now pointing to the newly created object
		// that has the same set of IDs, which causes the bindings to compound every time we switch a weapon.
		//
		// While an engine modification with a pull request to Epic would be the best way to solve this issue,
		// as it would remove the memory leak whenever a component that uses inputs gets created and deleted,
		// I don't want to modify engine for this recruitment task, so instead I'll just handle this locally
		// just for this equipment component by unregistering inputs manually
		APawn* OwnerAsPawn = Cast<APawn>(GetOwner());
		if (UInputComponent* InputComponent = OwnerAsPawn ? OwnerAsPawn->InputComponent : nullptr)
		{
			InputComponent->ClearBindingsForObject(WeaponGrantedAbilities[i]);
		}

		WeaponGrantedAbilities[i]->UnregisterComponent();
		WeaponGrantedAbilities[i]->DestroyComponent();
		WeaponGrantedAbilities.RemoveAt(i);
	}

	// Add new abilities
	if (NewWeapon != nullptr)
	{
		const TArray<TSubclassOf<UPlayerAbility>>& AbilityClasses = NewWeapon.GetDefaultObject()->GrantedAbilities;

		for (int32 i = 0; i < AbilityClasses.Num(); i++)
		{
			const TSubclassOf<UPlayerAbility>& AClass = AbilityClasses[i];
			if (AClass != nullptr)
			{
				FString ComponentName = FString::Printf(TEXT("WepAbility_%s_%d"), *AClass->GetName(), i);

				if (UPlayerAbility* AbilityComp = NewObject<UPlayerAbility>(GetOwner(), AClass, FName(*ComponentName)))
				{
					AbilityComp->GrantedByItem = NewWeapon;
					AbilityComp->RegisterComponent();
					AbilityComp->SetIsReplicated(true);
					AbilityComp->SetNetAddressable();
					WeaponGrantedAbilities.Add(AbilityComp);
				}
			}
		}
	}
}
