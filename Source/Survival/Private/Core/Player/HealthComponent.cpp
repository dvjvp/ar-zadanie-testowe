#include "Core/Player/HealthComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameModeBase.h"

UHealthComponent::UHealthComponent()
	: Super()
{
	SetIsReplicatedByDefault(true);
}

void UHealthComponent::DealDamage(float Damage)
{
	// Update health immediately on a client.
	// Server will send the correct data once it updates it itself
	// In case it disagrees with our version of damage being dealt, we'll take
	// the data from server

	const float PrevHealth = HealthPoints;
	HealthPoints = FMath::Max(0.0f, HealthPoints - Damage);
	
	OnHealthChanged.Broadcast(this, PrevHealth, HealthPoints);
	CheckIfDied(PrevHealth);
}

void UHealthComponent::RequestRespawn()
{
	if (ControllerToResetOnRespawn)
	{
		if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
		{
			GameMode->RestartPlayer(ControllerToResetOnRespawn);
		}
	}

	ControllerToResetOnRespawn = nullptr;
	GetOwner()->Destroy(true);
}

void UHealthComponent::CheckIfDied(float PreviousHealth)
{
	if (PreviousHealth > 0.0f && HealthPoints <= 0.0f)
	{
		OnDied.Broadcast(this);

		APawn* OwnerAsPawn = Cast<APawn>(GetOwner());
		ControllerToResetOnRespawn = OwnerAsPawn ? OwnerAsPawn->GetController() : nullptr;
		if (ControllerToResetOnRespawn)
		{
			ControllerToResetOnRespawn->Reset();
		}

		if (GetOwner()->HasAuthority())
		{
			GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &UHealthComponent::RequestRespawn, DelayBeforeRespawn);
		}
	}
}

void UHealthComponent::OnRep_HealthPoints(float PreviousValue)
{
	OnHealthChanged.Broadcast(this, PreviousValue, HealthPoints);
	CheckIfDied(PreviousValue);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UHealthComponent, HealthPoints);
}
