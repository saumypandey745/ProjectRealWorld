// ============================================================================
// IndianTrafficSpawner.cpp
// Project RealWorld — Indian Traffic System
// Realistic Indian traffic: autos, Marutis, bikes, trucks, tractors
// Time-of-day density: rush hour peak, quiet nights, busy markets
// ============================================================================

#include "World/IndianTrafficSpawner.h"
#include "Vehicles/RealWorldVehicleBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AIndianTrafficSpawner::AIndianTrafficSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;
}

void AIndianTrafficSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void AIndianTrafficSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Spawn check every 3 seconds
	SpawnCheckTimer += DeltaTime;
	if (SpawnCheckTimer >= 3.0f)
	{
		SpawnCheckTimer = 0.0f;
		if (SpawnedVehicles.Num() < MaxSpawnedVehicles)
		{
			TrySpawnVehicle();
		}
	}

	// Despawn check every 5 seconds
	DespawnCheckTimer += DeltaTime;
	if (DespawnCheckTimer >= 5.0f)
	{
		DespawnCheckTimer = 0.0f;
		DespawnFarVehicles();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// SPAWN LOGIC
// ─────────────────────────────────────────────────────────────────────────────
void AIndianTrafficSpawner::TrySpawnVehicle()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	// Don't over-spawn below density threshold
	if (FMath::FRand() > CurrentTrafficDensity) return;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return;

	// Find random navmesh point at spawn distance
	const FVector PlayerPos = PlayerPawn->GetActorLocation();
	float RandomAngle = FMath::FRandRange(0.0f, 2.0f * PI);
	float RandomDist  = FMath::FRandRange(SpawnRadius * 0.6f, SpawnRadius);
	FVector SpawnOffset = FVector(FMath::Cos(RandomAngle) * RandomDist,
	                               FMath::Sin(RandomAngle) * RandomDist, 0.0f);
	FVector SpawnLocation = PlayerPos + SpawnOffset;

	FNavLocation NavPoint;
	if (!NavSys->ProjectPointToNavigation(SpawnLocation, NavPoint, FVector(200.0f, 200.0f, 200.0f)))
	{
		return;
	}

	// Check we're not spawning in player's line of sight
	FHitResult LineHit;
	FCollisionQueryParams LineParams;
	LineParams.AddIgnoredActor(PlayerPawn);
	bool bVisible = !GetWorld()->LineTraceSingleByChannel(
		LineHit, PlayerPos, NavPoint.Location, ECC_Visibility, LineParams
	);
	if (bVisible && FVector::Dist(PlayerPos, NavPoint.Location) < SpawnRadius * 0.5f)
	{
		return; // Too close and visible — jarring pop-in
	}

	// Select vehicle class based on time
	float CurrentHour = 12.0f;
	// Get from TimeOfDay subsystem if available
	if (UWorld* World = GetWorld())
	{
		// TimeOfDaySubsystem integration would go here
	}

	TSubclassOf<ARealWorldVehicleBase> VehicleClass = SelectVehicleClass(CurrentHour);
	if (!VehicleClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

	// Spawn facing a random road direction
	FRotator SpawnRotation = FRotator(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);
	ARealWorldVehicleBase* NewVehicle = GetWorld()->SpawnActor<ARealWorldVehicleBase>(
		VehicleClass, NavPoint.Location, SpawnRotation, SpawnParams
	);

	if (NewVehicle)
	{
		SpawnedVehicles.Add(NewVehicle);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// DESPAWN FAR VEHICLES
// ─────────────────────────────────────────────────────────────────────────────
void AIndianTrafficSpawner::DespawnFarVehicles()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	const FVector PlayerPos = PlayerPawn->GetActorLocation();

	for (int32 i = SpawnedVehicles.Num() - 1; i >= 0; --i)
	{
		if (!SpawnedVehicles[i].IsValid())
		{
			SpawnedVehicles.RemoveAt(i);
			continue;
		}

		const float Dist = FVector::Dist(PlayerPos, SpawnedVehicles[i]->GetActorLocation());
		if (Dist > DespawnRadius)
		{
			SpawnedVehicles[i]->Destroy();
			SpawnedVehicles.RemoveAt(i);
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// VEHICLE CLASS SELECTION — District + Time aware
// ─────────────────────────────────────────────────────────────────────────────
TSubclassOf<ARealWorldVehicleBase> AIndianTrafficSpawner::SelectVehicleClass(float CurrentHour) const
{
	TArray<FIndianVehicleSpawnEntry> ValidEntries;

	for (const FIndianVehicleSpawnEntry& Entry : VehiclePool)
	{
		if (!Entry.VehicleClass) continue;

		// Time filter
		if (CurrentHour < Entry.MinHour || CurrentHour > Entry.MaxHour) continue;

		// District filter
		bool bDistrictOK = Entry.AllowedDistricts.Num() == 0;
		for (EIndianDistrict D : Entry.AllowedDistricts)
		{
			if (D == CurrentDistrict) { bDistrictOK = true; break; }
		}
		if (!bDistrictOK) continue;

		ValidEntries.Add(Entry);
	}

	if (ValidEntries.Num() == 0) return nullptr;

	// Weighted random selection
	float TotalWeight = 0.0f;
	for (const auto& E : ValidEntries) TotalWeight += E.SpawnWeight;

	float Roll = FMath::FRandRange(0.0f, TotalWeight);
	float Acc  = 0.0f;
	for (const auto& E : ValidEntries)
	{
		Acc += E.SpawnWeight;
		if (Roll <= Acc) return E.VehicleClass;
	}

	return ValidEntries.Last().VehicleClass;
}

// ─────────────────────────────────────────────────────────────────────────────
// TIME OF DAY TRAFFIC DENSITY (Indian rush hours)
// ─────────────────────────────────────────────────────────────────────────────
float AIndianTrafficSpawner::GetTrafficDensityForHour(float Hour) const
{
	// Indian traffic pattern:
	// Early morning 5-7: light — milkman, vendors
	// Morning rush 7:30-10: PEAK — office, school rush
	// Midday 10-12: moderate — delivery vehicles
	// Afternoon 12-4: lower — lunch time, heat
	// Evening rush 5-8: PEAK — office return, market
	// Late night 10-12: very light
	// Night 12-5: almost empty

	if (Hour < 5.0f)  return 0.05f;
	if (Hour < 7.0f)  return FMath::Lerp(0.05f, 0.3f, (Hour - 5.0f) / 2.0f);
	if (Hour < 10.0f) return FMath::Lerp(0.3f, 0.95f, (Hour - 7.0f) / 3.0f); // Rush!
	if (Hour < 12.0f) return FMath::Lerp(0.95f, 0.6f, (Hour - 10.0f) / 2.0f);
	if (Hour < 16.0f) return 0.5f;
	if (Hour < 17.0f) return FMath::Lerp(0.5f, 0.95f, (Hour - 16.0f)); // Pre-rush
	if (Hour < 20.0f) return 0.95f; // Evening PEAK
	if (Hour < 22.0f) return FMath::Lerp(0.95f, 0.2f, (Hour - 20.0f) / 2.0f);
	return 0.05f;
}

void AIndianTrafficSpawner::UpdateTrafficForTimeOfDay(float CurrentHour)
{
	CurrentTrafficDensity = GetTrafficDensityForHour(CurrentHour);

	// Adjust max vehicles based on time
	MaxSpawnedVehicles = FMath::RoundToInt(FMath::Lerp(5.0f, 20.0f, CurrentTrafficDensity));
}

void AIndianTrafficSpawner::SetDistrict(EIndianDistrict NewDistrict)
{
	CurrentDistrict = NewDistrict;

	// Adjust traffic density baseline by district type
	switch (NewDistrict)
	{
	case EIndianDistrict::Commercial:
	case EIndianDistrict::OldCity:
		MaxSpawnedVehicles = 20;
		break;
	case EIndianDistrict::Highway:
		MaxSpawnedVehicles = 15;
		break;
	case EIndianDistrict::Village:
	case EIndianDistrict::Farm:
		MaxSpawnedVehicles = 5;
		break;
	case EIndianDistrict::Beach:
	case EIndianDistrict::Forest:
		MaxSpawnedVehicles = 3;
		break;
	default:
		MaxSpawnedVehicles = 12;
		break;
	}
}
