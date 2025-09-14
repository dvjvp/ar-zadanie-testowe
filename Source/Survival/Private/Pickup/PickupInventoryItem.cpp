#include "Pickup/PickupInventoryItem.h"
#include "Inventory/InventoryComponent.h"

bool APickupInventoryItem::CanBePickedUp_Implementation(AActor* ByActor) const
{
	if (!Super::CanBePickedUp_Implementation(ByActor))
	{
		return false;
	}

	if (InventoryItem != nullptr)
	{
		if (APawn* AsPawn = Cast<APawn>(ByActor))
		{
			if (UInventoryComponent* Inventory = AsPawn->GetComponentByClass<UInventoryComponent>())
			{
				return Inventory->CanAddItems(InventoryItem);
			}
		}
	}

	return false;
}

void APickupInventoryItem::ApplyPickupEffect_Implementation(AActor* ToActor)
{
	if (InventoryItem != nullptr)
	{
		if (APawn* AsPawn = Cast<APawn>(ToActor))
		{
			if (UInventoryComponent* Inventory = AsPawn->GetComponentByClass<UInventoryComponent>())
			{
				Inventory->AddItems(InventoryItem, NumToAdd);
			}
		}
	}
}
