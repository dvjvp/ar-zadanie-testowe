#include "Core/Abilities/PlayerAbility.h"

void UPlayerAbility::TryToActivate()
{
	Rpc_RequestActivation();
	OnActivated(); // Local simulation
}

void UPlayerAbility::OnActivated_Implementation()
{

}

void UPlayerAbility::TryToCancel()
{
	Rpc_RequestCancelation();
	OnCanceled(EAbilityCancelReason::CanceledByGameCode); // Local simulation
}

void UPlayerAbility::OnCanceled_Implementation(EAbilityCancelReason Reason)
{

}

bool UPlayerAbility::CanBeActivated_Implementation()
{
	return true;
}

bool UPlayerAbility::CanBeCanceled_Implementation()
{
	return true;
}

bool UPlayerAbility::IsLocalCopy()
{
	const APawn* OwnerAsPawn = Cast<APawn>(GetOwner());
	return OwnerAsPawn && OwnerAsPawn->IsLocallyControlled();
}

void UPlayerAbility::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OnCanceled(EAbilityCancelReason::AbilityRemovedFromPlayer);
}

void UPlayerAbility::Rpc_RequestActivation_Implementation()
{
	if (CanBeActivated())
	{
		Rpc_AbilityActivated();
	}
	else
	{
		Rpc_DenyActivation();
	}
}

void UPlayerAbility::Rpc_DenyActivation_Implementation()
{
	OnCanceled(EAbilityCancelReason::AbortedByServer);
}

void UPlayerAbility::Rpc_AbilityActivated_Implementation()
{
	// Local copy is activated by the local simulation and doesn't wait for the server
	if (!IsLocalCopy())
	{
		OnActivated();
	}
}

void UPlayerAbility::Rpc_RequestCancelation_Implementation()
{
	if (CanBeCanceled())
	{
		Rpc_AbilityCanceled();
	}
}

void UPlayerAbility::Rpc_AbilityCanceled_Implementation()
{
	// Local copy is canceled by the local simulation and doesn't wait for the server
	if (!IsLocalCopy())
	{
		OnCanceled(EAbilityCancelReason::CanceledByGameCode);
	}
}

bool UPlayerAbility::IsSupportedForNetworking() const
{
	return GetIsReplicated();
}
