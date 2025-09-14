#include "Pickup/PickupSpawnerBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "Components/BillboardComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
APickupSpawnerBase::APickupSpawnerBase()
{
	USceneComponent* NewRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	NewRoot->SetupAttachment(RootComponent);

	SpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnLocation"));
	SpawnLocation->SetupAttachment(NewRoot);

	VisualizationComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("VisualizationComp"));
	VisualizationComp->SetupAttachment(NewRoot);
	VisualizationComp->SetRelativeLocation(FVector::UpVector * 10.0f);

	SetReplicates(true);
}

bool APickupSpawnerBase::TrySpawnPickup()
{
	if (SpawnedPickupActor.IsValid())
	{
		return false;
	}

	if (!HasAuthority())
	{
		return false;
	}

	SpawnedPickupActor = GetWorld()->SpawnActor<APickupBase>(PickupClass, SpawnLocation->GetComponentTransform());
	if (SpawnedPickupActor.IsValid())
	{
		SpawnedPickupActor->OnDestroyed.AddDynamic(this, &APickupSpawnerBase::OnSpawnedPickupDestroyed);
		
		// Reset variables used to time the vfx on remote clients
		ResetCooldownVFX();
	}

	return SpawnedPickupActor != nullptr;
}

void APickupSpawnerBase::OnSpawnedPickupDestroyed(AActor* DestroyedPickup)
{
	// Will be called only on server, because it's registered in TrySpawnPickup() which is server only

	// Calculate cooldown time before the next respawn
	const float& CooldownMin = TimeToRespawnMin;
	const float& CooldownMax = TimeToRespawnMax > TimeToRespawnMin ? TimeToRespawnMax : TimeToRespawnMin;

	const float SecondsUntilNextSpawn = FMath::RandRange(CooldownMin, CooldownMax);
	
	StartCooldownTimer(SecondsUntilNextSpawn);
}

void APickupSpawnerBase::StartCooldownTimer(float SecondsUntilNextSpawn)
{
	// Used to make the counter until the next spawn more precise on remote clients
	const float TimeAtNextSpawn = UGameplayStatics::GetGameState(this)->GetServerWorldTimeSeconds() + SecondsUntilNextSpawn;

	// Start cooldown until the next spawn
	GetWorld()->GetTimerManager().SetTimer(TimerUntilNextSpawn, this, &APickupSpawnerBase::OnTimerUntilNextSpawnCalled, SecondsUntilNextSpawn);

	// And show cooldown effect on all clients
	ShowOnCooldownStartVFX(SecondsUntilNextSpawn, TimeAtNextSpawn);
}

void APickupSpawnerBase::ShowOnCooldownStartVFX(float TotalCooldown, float ServerTimeAtCooldownEnd)
{
	Rep_ServerTimeAtNextSpawn = ServerTimeAtCooldownEnd;
	Rep_CurrentTotalCooldownTime = TotalCooldown;
}

void APickupSpawnerBase::ResetCooldownVFX()
{
	Rep_ServerTimeAtNextSpawn = -1.0f;
	Rep_CurrentTotalCooldownTime = -1.0f;
}

void APickupSpawnerBase::OnTimerUntilNextSpawnCalled()
{
	TimerUntilNextSpawn.Invalidate();

	// This function shouldn't be called not on the server,
	// but a sanity check won't hurt
	if (HasAuthority())
	{
		const bool SpawnedSuccessfully = TrySpawnPickup();
		if (!SpawnedSuccessfully)
		{
			// If something goes wrong, try again after a small delay
			StartCooldownTimer(1.0f);
		}
	}
}

float APickupSpawnerBase::GetCooldownProgress() const
{
	if (Rep_CurrentTotalCooldownTime <= 0.0f)
	{
		return 1.0f;
	}

	const float CurrentServerTime = UGameplayStatics::GetGameState(this)->GetServerWorldTimeSeconds();
	float RemainingTime = Rep_ServerTimeAtNextSpawn - CurrentServerTime;
	RemainingTime = RemainingTime > 0.0f ? RemainingTime : 0.0f;

	const float RemainingPercent = RemainingTime / Rep_CurrentTotalCooldownTime;
	return 1.0f - RemainingPercent;
}

bool APickupSpawnerBase::IsOnCooldown() const
{
	return Rep_CurrentTotalCooldownTime > 0 && Rep_ServerTimeAtNextSpawn > 0;
}

// Called when the game starts or when spawned
void APickupSpawnerBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		TrySpawnPickup();
	}
}

void APickupSpawnerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (TimerUntilNextSpawn.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerUntilNextSpawn);
	}
}

void APickupSpawnerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(APickupSpawnerBase, Rep_ServerTimeAtNextSpawn);
	DOREPLIFETIME(APickupSpawnerBase, Rep_CurrentTotalCooldownTime);
}

void APickupSpawnerBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (VisualizationComp && EditorIcon)
	{
		VisualizationComp->SetSprite(EditorIcon);
	}
}

