#include "UI/Game/GameUI.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/EquipmentComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "Core/Player/HealthComponent.h"
#include "Core/Player/SurvivalCharacter.h"

void UGameUI::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* LocalPlayerController = GetOwningPlayer();
	if (LocalPlayerController)
	{
		LocalPlayerController->OnPossessedPawnChanged.AddDynamic(this, &UGameUI::OnControlledPawnChanged);
	}

	OnControlledPawnChanged(nullptr, LocalPlayerController->GetPawn());
}

void UGameUI::OnControlledPawnChanged(APawn* PrevPawn, APawn* NewPawn)
{
	if (PrevPawn)
	{
		if (UEquipmentComponent* Equipment = PrevPawn->GetComponentByClass<UEquipmentComponent>())
		{
			Equipment->OnCurrentWeaponChanged.RemoveDynamic(this, &UGameUI::OnCurrentWeaponChanged);
		}
		if (UInventoryComponent* Inventory = PrevPawn->GetComponentByClass<UInventoryComponent>())
		{
			Inventory->OnInventoryChanged.RemoveDynamic(this, &UGameUI::OnInventoryModified);
		}
		if (UHealthComponent* Health = PrevPawn->GetComponentByClass<UHealthComponent>())
		{
			Health->OnHealthChanged.RemoveDynamic(this, &UGameUI::OnHealthChanged);
		}
	}

	const bool PlayerIsAliveAndInGame = Cast<ASurvivalCharacter>(NewPawn) != nullptr;
	OnPlayerStateChanged(PlayerIsAliveAndInGame);

	if (NewPawn)
	{
		if (UEquipmentComponent* Equipment = NewPawn->GetComponentByClass<UEquipmentComponent>())
		{
			Equipment->OnCurrentWeaponChanged.AddDynamic(this, &UGameUI::OnCurrentWeaponChanged);

			UpdateCurrentWeapon(Equipment->GetCurrentWeapon());
		}
		else
		{
			UpdateCurrentWeapon(nullptr);
		}

		if (UInventoryComponent* Inventory = NewPawn->GetComponentByClass<UInventoryComponent>())
		{
			Inventory->OnInventoryChanged.AddDynamic(this, &UGameUI::OnInventoryModified);
		}

		if (UHealthComponent* Health = NewPawn->GetComponentByClass<UHealthComponent>())
		{
			Health->OnHealthChanged.AddDynamic(this, &UGameUI::OnHealthChanged);
			// Immediately update the UI
			OnHealthChanged(Health, Health->GetHealthPoints(), Health->GetHealthPoints());
		}
	}
}

void UGameUI::OnCurrentWeaponChanged(UEquipmentComponent* Equipment, TSubclassOf<UInventoryWeaponDefinition> NewWeapon)
{
	UpdateCurrentWeapon(NewWeapon);
}

void UGameUI::OnInventoryModified(UInventoryComponent* Inventory, TSubclassOf<UInventoryItemDefinition> Item, int32 PrevCount, int32 NewCount)
{
	if (CurrentWeapon != nullptr)
	{
		if (Item == CurrentWeapon.GetDefaultObject()->AmmoType)
		{
			UpdateCurrentAmmo();
		}
	}
}

void UGameUI::OnHealthChanged(UHealthComponent* Health, float PreviousValue, float NewValue)
{
	if (HealthTextBlock)
	{
		const FString Message = FString::Printf(TEXT("HP : %d/%d"), static_cast<int32>(NewValue), GetDefaultHealth(Health));
		HealthTextBlock->SetText(FText::FromString(Message));
	}
}

void UGameUI::UpdateCurrentWeapon(TSubclassOf<UInventoryWeaponDefinition> NewWeapon)
{
	CurrentWeapon = NewWeapon;

	if (NewWeapon != nullptr)
	{
		WeaponImage->SetBrushFromTexture(NewWeapon.GetDefaultObject()->UiIcon);
	}
	else
	{
		WeaponImage->SetBrushFromTexture(NoWeaponTexture);
	}

	UpdateCurrentAmmo();
}

void UGameUI::UpdateCurrentAmmo()
{
	FString NewText = TEXT("");

	if (CurrentWeapon != nullptr)
	{
		APlayerController* LocalPlayerController = GetOwningPlayer();
		if (UInventoryComponent* Inventory = LocalPlayerController->GetPawn()->GetComponentByClass<UInventoryComponent>())
		{
			const int32 NumAmmo = Inventory->GetItemCount(CurrentWeapon.GetDefaultObject()->AmmoType);
			NewText = FString::FromInt(NumAmmo);
		}
	}

	AmmoCountTextBlock->SetText(FText::FromString(NewText));
}

int32 UGameUI::GetDefaultHealth(UHealthComponent* Health) const
{
	// Defaults might be overridden by the owner, so let's get them from the default object for the owner

	const UClass* OwnerClass = Health->GetOwner()->GetClass();
	if (OwnerClass)
	{
		const UObject* DefaultOwnerObject = OwnerClass->GetDefaultObject(false);
		const AActor* DefaultOwnerAsActor = Cast<AActor>(DefaultOwnerObject);
		if (DefaultOwnerAsActor)
		{
			const UHealthComponent* DefaultHealth = DefaultOwnerAsActor->GetComponentByClass<UHealthComponent>();
			if (DefaultHealth)
			{
				return static_cast<int32>(DefaultHealth->GetHealthPoints());
			}
		}
	}

	return 0;
}

void UGameUI::BeginDestroy()
{
	Super::BeginDestroy();

	APlayerController* LocalPlayerController = GetOwningPlayer();
	if (LocalPlayerController)
	{
		LocalPlayerController->OnPossessedPawnChanged.RemoveDynamic(this, &UGameUI::OnControlledPawnChanged);

		if (APawn* Pawn = LocalPlayerController->GetPawn())
		{
			if (UEquipmentComponent* Equipment = Pawn->GetComponentByClass<UEquipmentComponent>())
			{
				Equipment->OnCurrentWeaponChanged.RemoveDynamic(this, &UGameUI::OnCurrentWeaponChanged);
			}

			if (UInventoryComponent* Inventory = Pawn->GetComponentByClass<UInventoryComponent>())
			{
				Inventory->OnInventoryChanged.RemoveDynamic(this, &UGameUI::OnInventoryModified);
			}
		}
	}
}
