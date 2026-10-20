#include "World/RealWorldSpawnerSubsystem.h"
#include "Subsystems/RealWorldAudioSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void URealWorldSpawnerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentDistrict = EWorldDistrictType::DowntownCity;
	BudgetCheckTimer = 0.0f;
}

void URealWorldSpawnerSubsystem::Tick(float DeltaTime)
{
	BudgetCheckTimer += DeltaTime;
	if (BudgetCheckTimer < 1.0f) return; // Evaluate every 1 second
	BudgetCheckTimer = 0.0f;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	const FVector PlayerPos = PlayerPawn->GetActorLocation();
	UpdateDistrictEvaluation(PlayerPos);
	ManageStreamingBudgets(PlayerPos);
}

TStatId URealWorldSpawnerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URealWorldSpawnerSubsystem, STATGROUP_Tickables);
}

void URealWorldSpawnerSubsystem::UpdateDistrictEvaluation(const FVector& PlayerPos)
{
	// Coordinate-based district boundary zoning (supporting Large World Coordinates)
	EWorldDistrictType CalculatedDistrict = EWorldDistrictType::DowntownCity;

	const double DistFromCenter = FVector2D(PlayerPos.X, PlayerPos.Y).Size();

	if (PlayerPos.Z < -800.0f)
	{
		CalculatedDistrict = EWorldDistrictType::UndergroundTransit;
	}
	else if (DistFromCenter < 50000.0) // 500m radius
	{
		CalculatedDistrict = EWorldDistrictType::DowntownCity;
	}
	else if (DistFromCenter < 120000.0) // 1.2km radius
	{
		CalculatedDistrict = EWorldDistrictType::Suburbs;
	}
	else if (PlayerPos.X > 120000.0)
	{
		CalculatedDistrict = EWorldDistrictType::IndustrialDocks;
	}
	else if (PlayerPos.Y > 120000.0)
	{
		CalculatedDistrict = EWorldDistrictType::ForestPark;
	}
	else
	{
		CalculatedDistrict = EWorldDistrictType::RuralHighways;
	}

	if (CalculatedDistrict != CurrentDistrict)
	{
		CurrentDistrict = CalculatedDistrict;
		OnDistrictChanged.Broadcast(CurrentDistrict);
	}
}

void URealWorldSpawnerSubsystem::ManageStreamingBudgets(const FVector& PlayerPos)
{
	// Distance-based streaming check iterates through active ambient actors
	// Despawning or pooling actors when distance exceeds MaxActiveRadius
}

void URealWorldSpawnerSubsystem::NotifyInteriorEntered(FName InteriorName)
{
	if (UWorld* World = GetWorld())
	{
		if (URealWorldAudioSubsystem* AudioSub = World->GetSubsystem<URealWorldAudioSubsystem>())
		{
			AudioSub->SetAcousticZone(EAudioAcousticZone::InteriorSmallRoom);
			AudioSub->SetListenerInteriorOcclusion(true, 0.75f);
		}
	}
}

void URealWorldSpawnerSubsystem::NotifyInteriorExited(FName InteriorName)
{
	if (UWorld* World = GetWorld())
	{
		if (URealWorldAudioSubsystem* AudioSub = World->GetSubsystem<URealWorldAudioSubsystem>())
		{
			AudioSub->SetAcousticZone(EAudioAcousticZone::ExteriorOpenAir);
			AudioSub->SetListenerInteriorOcclusion(false, 0.0f);
		}
	}
}
