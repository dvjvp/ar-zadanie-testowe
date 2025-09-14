#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup/PickupBase.h"
#include "PickupSpawnerBase.generated.h"

UCLASS(Blueprintable)
class SURVIVAL_API APickupSpawnerBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupSpawnerBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(/*Server, Reliable,*/ Category = "Pickup")
	bool TrySpawnPickup();
	
	UFUNCTION()
	void OnSpawnedPickupDestroyed(AActor* DestroyedPickup);

	void StartCooldownTimer(float SecondsUntilNextSpawn);

	void ShowOnCooldownStartVFX(float TotalCooldown, float ServerTimeAtCooldownEnd);
	void ResetCooldownVFX();

	UFUNCTION()
	void OnTimerUntilNextSpawnCalled();

	UFUNCTION(BlueprintPure)
	float GetCooldownProgress() const;

	UFUNCTION(BlueprintPure)
	bool IsOnCooldown() const;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	
	UPROPERTY(EditAnywhere, Category = "Pickup")
	TSubclassOf<APickupBase> PickupClass;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	class UTexture2D* EditorIcon = nullptr;

	UPROPERTY()
	TWeakObjectPtr<APickupBase> SpawnedPickupActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USceneComponent* SpawnLocation = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBillboardComponent* VisualizationComp = nullptr;

	UPROPERTY(EditAnywhere, Category = "Pickup|Spawn", meta=(Units="s"))
	float TimeToRespawnMin = 1.0f;

	// If smaller than TimeToRespawnMin, spawning in the same intervals, equal to TimeToRespawnMin.
	UPROPERTY(EditAnywhere, Category = "Pickup|Spawn", meta=(Units="s"))
	float TimeToRespawnMax = 0.0f;

	// We're using replicated variables to control vfx here, so that if someone joins in mid-cooldown
	// they won't be missing information about the next spawn from a missed rpc
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Replication")
	float Rep_ServerTimeAtNextSpawn = -1.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Replication")
	float Rep_CurrentTotalCooldownTime = -1.0f;

	FTimerHandle TimerUntilNextSpawn;	
};
