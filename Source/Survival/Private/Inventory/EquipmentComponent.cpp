#include "Inventory/EquipmentComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/InventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"
#include "Core/Abilities/PlayerAbility.h"

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
	CurrentWeapon = NewWeapon;
	OnCurrentWeaponChanged.Broadcast(this, CurrentWeapon);

	// Remove all previously granted abilities
	for (int32 i = WeaponGrantedAbilities.Num() - 1; i >= 0; i--)
	{
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
