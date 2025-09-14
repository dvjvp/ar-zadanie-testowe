#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "PickupBase.generated.h"

UCLASS(Blueprintable)
class SURVIVAL_API APickupBase : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupBase();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Pickup")
	bool CanBePickedUp(AActor* ByActor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
	void ApplyPickupEffect(AActor* ToActor);
	

	UFUNCTION()
	void OnOverlap(AActor* ThisActor, AActor* OtherActor);

	UFUNCTION()
	void PickUp(AActor* PickedByActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, /*Client,*/ Category = "Pickup|VFX")
	void OnPickedUpEffects();
	
	
protected:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PickupIconComp = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class USphereComponent* CollisionComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	class UNiagaraSystem* OnPickedUpVFX = nullptr;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	class UStaticMesh* PickupIconMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	class UMaterialInterface* PickupIconMaterial = nullptr;

};
