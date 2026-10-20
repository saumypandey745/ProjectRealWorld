#include "AI/RealWorldCrowdManager.h"
#include "AI/RealWorldNPCCharacter.h"
#include "Platform/RealWorldPlatformSubsystem.h"
#include "Engine/World.h"

ARealWorldCrowdManager::ARealWorldCrowdManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.5f; // Budgeted 2 Hz manager tick

	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("CrowdRoot"));
	SetRootComponent(RootComp);

	TargetCrowdCount = 24;
	StoreEntrancePoint = FVector(2000.0f, 950.0f, 16.0f);
	StoreShelfPoint = FVector(2200.0f, 1550.0f, 16.0f);
	StoreCounterPoint = FVector(1650.0f, 1450.0f, 16.0f);
	StoreDispatchTimer = 10.0f;
}

void ARealWorldCrowdManager::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (URealWorldPlatformSubsystem* PlatformSub = GI->GetSubsystem<URealWorldPlatformSubsystem>())
		{
			TargetCrowdCount = PlatformSub->GetScaledCrowdBudget();
		}
	}

	SpawnCityBlockCrowd();
}

void ARealWorldCrowdManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Periodic dispatch of ambient pedestrians into the storefront
	StoreDispatchTimer -= DeltaTime;
	if (StoreDispatchTimer <= 0.0f)
	{
		StoreDispatchTimer = FMath::FRandRange(15.0f, 25.0f);
		DispatchCivilianToStore();
	}
}

void ARealWorldCrowdManager::SpawnCityBlockCrowd()
{
	UWorld* World = GetWorld();
	if (!World) return;

	ActiveCrowdMembers.Empty();

	// Define 24 structured spawn positions across sidewalks, benches, crosswalks, and store
	const TArray<ENPCArchetype> ArchetypeDistribution = {
		ENPCArchetype::ShopWorker, ENPCArchetype::ShopWorker, // Dedicated store clerks inside
		ENPCArchetype::CivilianMale, ENPCArchetype::CivilianFemale,
		ENPCArchetype::Business, ENPCArchetype::Casual,
		ENPCArchetype::Police, ENPCArchetype::Security,
		ENPCArchetype::CivilianMale, ENPCArchetype::CivilianFemale,
		ENPCArchetype::Business, ENPCArchetype::Casual,
		ENPCArchetype::CivilianMale, ENPCArchetype::CivilianFemale,
		ENPCArchetype::MunicipalWorker, ENPCArchetype::Casual,
		ENPCArchetype::CivilianMale, ENPCArchetype::CivilianFemale,
		ENPCArchetype::Business, ENPCArchetype::Casual,
		ENPCArchetype::CivilianMale, ENPCArchetype::CivilianFemale,
		ENPCArchetype::CivilianMale, ENPCArchetype::CivilianFemale
	};

	const float NorthY = 820.0f;
	const float SouthY = -820.0f;
	const int32 CountToSpawn = FMath::Clamp(TargetCrowdCount, 4, ArchetypeDistribution.Num());

	for (int32 i = 0; i < CountToSpawn; ++i)
	{
		FVector SpawnLoc = FVector::ZeroVector;
		FRotator SpawnRot = FRotator::ZeroRotator;
		ENPCArchetype Arch = ArchetypeDistribution[i];

		if (Arch == ENPCArchetype::ShopWorker)
		{
			// Spawn inside the convenience store behind counter
			SpawnLoc = FVector(1600.0f + (i * 80.0f), 1450.0f, 26.0f);
			SpawnRot = FRotator(0.0f, -90.0f, 0.0f);
		}
		else if (i % 2 == 0)
		{
			// North sidewalk positions
			const float OffsetX = -4500.0f + (i * 420.0f);
			SpawnLoc = FVector(OffsetX, NorthY, 21.0f);
			SpawnRot = FRotator(0.0f, (i % 4 == 0) ? 0.0f : 180.0f, 0.0f);
		}
		else
		{
			// South sidewalk positions
			const float OffsetX = -4500.0f + (i * 420.0f);
			SpawnLoc = FVector(OffsetX, SouthY, 21.0f);
			SpawnRot = FRotator(0.0f, (i % 4 == 1) ? 180.0f : 0.0f, 0.0f);
		}

		FActorSpawnParameters P;
		P.Owner = this;

		ARealWorldNPCCharacter* NPC = World->SpawnActor<ARealWorldNPCCharacter>(ARealWorldNPCCharacter::StaticClass(), SpawnLoc, SpawnRot, P);
		if (NPC)
		{
			NPC->SetArchetype(Arch);
			NPC->NPCName = FText::FromString(FString::Printf(TEXT("Citizen_%02d"), i + 1));
			ActiveCrowdMembers.Add(NPC);
		}
	}
}

void ARealWorldCrowdManager::DispatchCivilianToStore()
{
	// Find a walking civilian near the north sidewalk to send into the store
	for (ARealWorldNPCCharacter* NPC : ActiveCrowdMembers)
	{
		if (NPC && NPC->CurrentState == ENPCState::WALKING && NPC->Archetype != ENPCArchetype::ShopWorker && NPC->Archetype != ENPCArchetype::Police)
		{
			const float DistToStore = FVector::Dist(NPC->GetActorLocation(), StoreEntrancePoint);
			if (DistToStore < 3500.0f)
			{
				NPC->StartStoreVisitRoutine(StoreEntrancePoint, StoreShelfPoint, StoreCounterPoint);
				break;
			}
		}
	}
}

void ARealWorldCrowdManager::AlertNearbyCrowd(const FVector& ThreatOrigin, float ThreatRadius)
{
	for (ARealWorldNPCCharacter* NPC : ActiveCrowdMembers)
	{
		if (NPC && NPC->CurrentState != ENPCState::DEAD)
		{
			const float Dist = FVector::Dist(NPC->GetActorLocation(), ThreatOrigin);
			if (Dist <= ThreatRadius)
			{
				NPC->ReactToDanger(ThreatOrigin);
			}
		}
	}
}
