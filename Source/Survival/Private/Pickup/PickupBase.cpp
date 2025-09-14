#include "Pickup/PickupBase.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"


// Sets default values
APickupBase::APickupBase()
{
	OnActorBeginOverlap.AddDynamic(this, &APickupBase::OnOverlap);

	USceneComponent* NewRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	NewRoot->SetupAttachment(RootComponent);

	PickupIconComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupIconComp"));
	PickupIconComp->SetupAttachment(NewRoot);
	PickupIconComp->SetRelativeLocation(FVector::ZeroVector);

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->SetupAttachment(NewRoot);
	CollisionComp->SetRelativeLocation(FVector::ZeroVector);

	SetReplicates(true);
}

void APickupBase::ApplyPickupEffect_Implementation(AActor* ToActor)
{
	const FString LogMsg = FString::Printf(TEXT("[") TEXT(__FUNCTION__) TEXT("] called on %s by %s."), *this->GetName(), *ToActor->GetName());
	UE_LOG(LogTemp, Log, TEXT("%s"), *LogMsg);
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Orange, LogMsg);
}

void APickupBase::OnOverlap(AActor* ThisActor, AActor* OtherActor)
{
	if (CanBePickedUp(OtherActor))
	{
		PickUp(OtherActor);
	}
}

void APickupBase::PickUp(AActor* PickedByActor)
{
	APawn* AsPawn = Cast<APawn>(PickedByActor);
	const bool PickedByLocalPawn = AsPawn && AsPawn->IsLocallyControlled();
	if (PickedByLocalPawn)
	{
		// Client implementation for local client. Hide the pickup to show it's been picked up to
		// to better hide any delay between client and server. We don't need to do it for remote pawns
		// since if we're getting information about them moving into location where they'd collide with a pickup
		// from the server, we would also get a message from the server about the pickup getting destroyed.
		SetActorHiddenInGame(true);
		OnPickedUpEffects(); // Show pick up VFX immediately, don't wait for confirmation from server
	}
	else
	{
		// Remote client implementation. We don't need to do anything here.
	}

	// For local non-host players we want to apply the effect immediately to make the game more responsive
	// If it results with them trying to do something they're not supposed to, the server will prevent them
	// from doing that anyway
	if (HasAuthority() || PickedByLocalPawn)
	{
		ApplyPickupEffect(PickedByActor);
	}

	if (HasAuthority())
	{
		// Server implementation. Actually perform logic of applying pickup effect and destroying it.
		Destroy();
	}
}

void APickupBase::OnPickedUpEffects_Implementation()
{
	if (OnPickedUpVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, OnPickedUpVFX, GetActorLocation());
	}
}

void APickupBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		// When local player grabs a pickup, we're hiding it and showing the VFX there to decrease perceived latency.
		// So we only need to play the VFX on when this object is picked up by another player
		if (!IsHidden())
		{
			OnPickedUpEffects();
		}
	}
}

void APickupBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (PickupIconMesh && PickupIconComp && PickupIconMaterial)
	{
		PickupIconComp->SetStaticMesh(PickupIconMesh);
		
		const int32 NumMaterialSlots = PickupIconComp->GetNumMaterials();
		for (int32 i = 0; i < NumMaterialSlots; i++)
		{
			PickupIconComp->SetMaterial(i, PickupIconMaterial);
		}
	}
}

bool APickupBase::CanBePickedUp_Implementation(AActor* ByActor) const
{
	// Only characters can grab pick ups
	return Cast<ACharacter>(ByActor) != nullptr;
}
