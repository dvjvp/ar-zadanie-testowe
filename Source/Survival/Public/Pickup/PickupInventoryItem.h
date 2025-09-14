#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "Inventory/InventoryItemDefinition.h"
#include "PickupInventoryItem.generated.h"

UCLASS()
class SURVIVAL_API APickupInventoryItem : public APickupBase
{
	GENERATED_BODY()
public:
	
	virtual bool CanBePickedUp_Implementation(AActor* ByActor) const override;
	virtual void ApplyPickupEffect_Implementation(AActor* ToActor) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UInventoryItemDefinition> InventoryItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumToAdd = 1;
};
