#pragma once

#include "CoreMinimal.h"
#include "PlayerAbility.generated.h"

UENUM(BlueprintType)
enum class EAbilityCancelReason : uint8
{
	CanceledByGameCode,
	AbortedByServer,
	AbilityRemovedFromPlayer
};

 UCLASS(Blueprintable, Abstract)
class SURVIVAL_API UPlayerAbility : public UActorComponent
{
	GENERATED_BODY()
public:

	// Send request to server to try to activate the ability
	// and start activating it locally to decrease latency
	UFUNCTION(BlueprintCallable)
	void TryToActivate();

	// Actual gameplay code of the ability
	UFUNCTION(BlueprintNativeEvent)
	void OnActivated();

	UFUNCTION(BlueprintCallable)
	void TryToCancel();

	// Called when the ability ends or is interrupted
	UFUNCTION(BlueprintNativeEvent)
	void OnCanceled(EAbilityCancelReason Reason);

	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
	bool CanBeActivated();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
	bool CanBeCanceled();

	// Returns true if this ability exists on a pawn that is controlled by the local client machine
	UFUNCTION(BlueprintPure)
	bool IsLocalCopy();

protected:
	// Internal request from client to server to activate ability
	UFUNCTION(Server, Reliable)
	void Rpc_RequestActivation();

	// If server decides the ability can't be activated, it sends
	// this message to the client as a correction to abort its local simulation
	UFUNCTION(Client, Reliable)
	void Rpc_DenyActivation();

	// If server confirms that the ability is being activated,
	// it uses this message to replicate this behavior to other clients
	UFUNCTION(NetMulticast, Reliable)
	void Rpc_AbilityActivated();

	// Sent from client to server when the ability ends by the gameplay code
	UFUNCTION(Server, Reliable)
	void Rpc_RequestCancelation();

	// Sent from server to all clients to confirm that the ability has been canceled
	UFUNCTION(NetMulticast, Reliable)
	void Rpc_AbilityCanceled();

protected:

	UFUNCTION(BlueprintPure)
	class APlayerController* GetOwnersPlayerController() const;

public:
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<class UInventoryItemDefinition> GrantedByItem;

	virtual bool IsSupportedForNetworking() const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

 };
