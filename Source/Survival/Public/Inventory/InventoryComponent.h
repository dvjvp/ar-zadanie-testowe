#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryItemDefinition.h"
#include "InventoryComponent.generated.h"

class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnInventoryModified, UInventoryComponent*, Inventory, TSubclassOf<UInventoryItemDefinition>, Item, int32, PrevCount, int32, NewCount);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVAL_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(const TSubclassOf<UInventoryItemDefinition>& InventoryItem) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(const TSubclassOf<UInventoryItemDefinition>& InventoryItem) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool CanAddItems(const TSubclassOf<UInventoryItemDefinition>& InventoryItem) const;

	// Returns how many were actually added
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItems(const TSubclassOf<UInventoryItemDefinition>& InventoryItem, int32 Count);

	// Returns how many were actually removed
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveItems(const TSubclassOf<UInventoryItemDefinition>& InventoryItem, int32 Count);

	template<typename TItemType>
	TArray<TSubclassOf<TItemType>> GetAllItemsOfType()
	{
		TArray<TSubclassOf<TItemType>> Results;

		TArray<TSubclassOf<UInventoryItemDefinition>> Keys;
		InventoryInternal.GetKeys(Keys);

		for (const auto& Key : Keys)
		{
			if (Key.Get()->IsChildOf(TItemType::StaticClass()))
			{
				Results.Add(Key.Get());
			}
		}

		return Results;
	}

public:

	UPROPERTY(BlueprintAssignable)
	FOnInventoryModified OnInventoryChanged;

protected:

	UPROPERTY()
	TMap<TSubclassOf<UInventoryItemDefinition>, int32> InventoryInternal;
};
