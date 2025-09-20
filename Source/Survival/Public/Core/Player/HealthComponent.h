#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHealthChanged, UHealthComponent*, Caller, float, PreviousHealth, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterDied, UHealthComponent*, Caller);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVAL_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UHealthComponent();

	float GetHealthPoints() const { return HealthPoints; }

	UFUNCTION(BlueprintCallable)
	void DealDamage(float Damage);

	UFUNCTION(BlueprintCallable)
	void RequestRespawn();

	UPROPERTY(BlueprintAssignable)
	FHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FCharacterDied OnDied;

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void CheckIfDied(float PreviousHealth);

	UFUNCTION()
	void OnRep_HealthPoints(float PreviousValue);

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, ReplicatedUsing = "OnRep_HealthPoints")
	float HealthPoints = 100.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float DelayBeforeRespawn = 7.0f;

	UPROPERTY()
	FTimerHandle RespawnTimer;

	UPROPERTY()
	AController* ControllerToResetOnRespawn = nullptr;
};
