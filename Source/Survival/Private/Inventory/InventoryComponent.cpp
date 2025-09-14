#include "Inventory/InventoryComponent.h"


// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{

	// ...
}

bool UInventoryComponent::HasItem(const TSubclassOf<UInventoryItemDefinition>& InventoryItem) const
{
	return InventoryInternal.Contains(InventoryItem);
}

int32 UInventoryComponent::GetItemCount(const TSubclassOf<UInventoryItemDefinition>& InventoryItem) const
{
	if (const int32* Count = InventoryInternal.Find(InventoryItem))
	{
		return *Count;
	}
	else
	{
		return 0;
	}
}

bool UInventoryComponent::CanAddItems(const TSubclassOf<UInventoryItemDefinition>& InventoryItem) const
{
	const UInventoryItemDefinition* Definition = InventoryItem.GetDefaultObject();
	if (!Definition)
	{
		return false;
	}

	const int32 CurrentCount = GetItemCount(InventoryItem);
	return CurrentCount < Definition->MaxNumInInventory;
}

int32 UInventoryComponent::AddItems(const TSubclassOf<UInventoryItemDefinition>& InventoryItem, int32 AddCount)
{
	check(AddCount >= 0);

	const UInventoryItemDefinition* Definition = InventoryItem.GetDefaultObject();
	if (!Definition)
	{
		return 0;
	}

	int32& CurrCount = InventoryInternal.FindOrAdd(InventoryItem);

	const int32 PrevCount = CurrCount;

	const int32 NumToAdd = FMath::Min(AddCount, Definition->MaxNumInInventory - CurrCount);
	CurrCount += NumToAdd;

	OnInventoryChanged.Broadcast(this, InventoryItem, PrevCount, CurrCount);

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, FString::Printf(TEXT("Added %d x '%s' to inventory."), NumToAdd, *InventoryItem.Get()->GetName()));

	return NumToAdd;
}

int32 UInventoryComponent::RemoveItems(const TSubclassOf<UInventoryItemDefinition>& InventoryItem, int32 RemoveCount)
{
	check(RemoveCount >= 0);

	const UInventoryItemDefinition* Definition = InventoryItem.GetDefaultObject();
	if (!Definition)
	{
		return 0;
	}

	if (int32* Count = InventoryInternal.Find(InventoryItem))
	{
		const int32 PrevCount = *Count;

		const int32 NumToRemove = FMath::Min(RemoveCount, *Count);
		*Count -= NumToRemove;

		const int32 NewCount = *Count;

		if (*Count <= 0)
		{
			InventoryInternal.Remove(InventoryItem);
		}

		OnInventoryChanged.Broadcast(this, InventoryItem, PrevCount, NewCount);

		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, FString::Printf(TEXT("Removed %d x '%s' to inventory."), NumToRemove, *InventoryItem.Get()->GetName()));

		return NumToRemove;
	}
	else
	{
		return 0;
	}
}

