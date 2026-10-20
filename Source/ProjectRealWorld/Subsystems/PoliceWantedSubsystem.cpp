#include "Subsystems/PoliceWantedSubsystem.h"
#include "Core/RealWorldPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UPoliceWantedSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	WantedLevel = 0;
	CurrentPursuitState = EPursuitState::Calm;
	CrimeSeverityAccumulator = 0.0f;
	bPlayerInSight = false;
	EvasionTimer = 0.0f;
	CurrentSearchRadius = 3500.0f;
	MaxSearchRadius = 12000.0f;
	DispatchArrivalTimer = 0.0f;
	bDispatchActive = false;
}

void UPoliceWantedSubsystem::Tick(float DeltaTime)
{
	if (WantedLevel == 0) return;

	// 1. Dispatch response countdown
	if (bDispatchActive)
	{
		DispatchArrivalTimer -= DeltaTime;
		if (DispatchArrivalTimer <= 0.0f)
		{
			bDispatchActive = false;
			SpawnNaturalPolicePatrol(LastKnownPlayerLocation);
		}
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	const FVector PlayerLoc = PlayerPawn ? PlayerPawn->GetActorLocation() : LastKnownPlayerLocation;
	const float DistToSearchCenter = FVector::Dist(PlayerLoc, LastKnownPlayerLocation);

	// 2. State-driven pursuit & evasion progression
	if (bPlayerInSight)
	{
		CurrentPursuitState = EPursuitState::ActivePursuit;
		EvasionTimer = BaseEvasionDuration * WantedLevel;
		CurrentSearchRadius = 2500.0f;
	}
	else
	{
		// Suspect lost from visual sight
		if (CurrentPursuitState == EPursuitState::ActivePursuit)
		{
			CurrentPursuitState = EPursuitState::SearchingLastKnownArea;
		}

		// Search cordon expands gradually outward
		CurrentSearchRadius = FMath::Min(MaxSearchRadius, CurrentSearchRadius + (65.0f * DeltaTime));

		// Check if suspect has broken the perimeter cordon
		if (DistToSearchCenter > CurrentSearchRadius)
		{
			// Player has successfully exited the search zone -> Cooldown phase active
			CurrentPursuitState = EPursuitState::Cooldown;
			EvasionTimer -= DeltaTime;

			if (EvasionTimer <= 0.0f)
			{
				ClearWantedLevel();
				return;
			}
		}
		else
		{
			// Suspect is still inside search cordon: investigation active, evasion pauses
			CurrentPursuitState = EPursuitState::SearchingLastKnownArea;
			// Passive evidence decay inside zone
			EvasionTimer -= (DeltaTime * 0.25f);
			if (EvasionTimer <= 0.0f)
			{
				ClearWantedLevel();
				return;
			}
		}

		OnSearchAreaChanged.Broadcast(LastKnownPlayerLocation, CurrentSearchRadius);
	}

	OnWantedStateUpdated.Broadcast(WantedLevel, CurrentPursuitState);
}

TStatId UPoliceWantedSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UPoliceWantedSubsystem, STATGROUP_Tickables);
}

void UPoliceWantedSubsystem::ReportCrime(ECrimeType Crime, FVector Location, AActor* Perpetrator, bool bDirectPoliceSight)
{
	LastKnownPlayerLocation = Location;

	RecalculateThreatLevel(Crime);

	if (bDirectPoliceSight)
	{
		bPlayerInSight = true;
		CurrentPursuitState = EPursuitState::ActivePursuit;
		EvasionTimer = BaseEvasionDuration * WantedLevel;
		CurrentSearchRadius = 2500.0f;
	}
	else
	{
		// Non-omniscient: civilian report calls dispatch with realistic arrival delay
		bPlayerInSight = false;
		CurrentPursuitState = EPursuitState::Investigating;
		EvasionTimer = (BaseEvasionDuration * WantedLevel) + 15.0f;
		CurrentSearchRadius = 3200.0f + (WantedLevel * 800.0f);

		DispatchArrivalTimer = FMath::FRandRange(6.0f, 10.0f);
		bDispatchActive = true;
		OnPoliceDispatched.Broadcast(Location);
	}

	// Update PlayerState for HUD reflection
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ARealWorldPlayerState* PS = PC->GetPlayerState<ARealWorldPlayerState>())
		{
			PS->SetWantedLevel(WantedLevel);
		}
	}

	OnWantedStateUpdated.Broadcast(WantedLevel, CurrentPursuitState);
	OnSearchAreaChanged.Broadcast(LastKnownPlayerLocation, CurrentSearchRadius);
}

void UPoliceWantedSubsystem::UpdateSuspectVisual(FVector CurrentLocation)
{
	LastKnownPlayerLocation = CurrentLocation;
	bPlayerInSight = true;
	CurrentPursuitState = EPursuitState::ActivePursuit;
	EvasionTimer = BaseEvasionDuration * WantedLevel;
	CurrentSearchRadius = 2000.0f;

	OnSearchAreaChanged.Broadcast(LastKnownPlayerLocation, CurrentSearchRadius);
}

void UPoliceWantedSubsystem::LoseSuspectVisual()
{
	bPlayerInSight = false;
	CurrentPursuitState = EPursuitState::SearchingLastKnownArea;
}

void UPoliceWantedSubsystem::ClearWantedLevel()
{
	WantedLevel = 0;
	CurrentPursuitState = EPursuitState::Calm;
	CrimeSeverityAccumulator = 0.0f;
	bPlayerInSight = false;
	bDispatchActive = false;
	EvasionTimer = 0.0f;

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ARealWorldPlayerState* PS = PC->GetPlayerState<ARealWorldPlayerState>())
		{
			PS->SetWantedLevel(0);
		}
	}

	OnWantedStateUpdated.Broadcast(0, EPursuitState::Calm);
}

bool UPoliceWantedSubsystem::CanArrestPlayer(FVector OfficerLocation, bool bPlayerArmed) const
{
	if (bPlayerArmed || WantedLevel > 2)
	{
		// Lethal force authorized for armed suspects or high wanted levels
		return false;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return false;

	const float Distance = FVector::Dist(OfficerLocation, PlayerPawn->GetActorLocation());
	return Distance <= 400.0f; // Within 4 meters
}

void UPoliceWantedSubsystem::TriggerPlayerArrest()
{
	CurrentPursuitState = EPursuitState::ArrestingSuspect;
	OnPlayerArrested.Broadcast();
	ClearWantedLevel();
}

void UPoliceWantedSubsystem::RecalculateThreatLevel(ECrimeType NewCrime)
{
	float AddedSeverity = 0.2f;
	int32 MinimumLevel = 1;

	switch (NewCrime)
	{
	case ECrimeType::MinorTheft:
	case ECrimeType::RecklessDriving:
		AddedSeverity = 0.25f;
		MinimumLevel = 1;
		break;
	case ECrimeType::Assault:
		AddedSeverity = 0.5f;
		MinimumLevel = 2;
		break;
	case ECrimeType::GrandTheftAuto:
		AddedSeverity = 0.6f;
		MinimumLevel = 2;
		break;
	case ECrimeType::DischargingFirearm:
		AddedSeverity = 0.9f;
		MinimumLevel = 3;
		break;
	case ECrimeType::Homicide:
		AddedSeverity = 1.5f;
		MinimumLevel = 4;
		break;
	}

	CrimeSeverityAccumulator += AddedSeverity;

	int32 CalculatedLevel = MinimumLevel;
	if (CrimeSeverityAccumulator >= 3.0f)
	{
		CalculatedLevel = 5;
	}
	else if (CrimeSeverityAccumulator >= 2.0f)
	{
		CalculatedLevel = FMath::Max(CalculatedLevel, 4);
	}
	else if (CrimeSeverityAccumulator >= 1.2f)
	{
		CalculatedLevel = FMath::Max(CalculatedLevel, 3);
	}

	WantedLevel = FMath::Clamp(CalculatedLevel, 1, 5);
}

void UPoliceWantedSubsystem::SpawnNaturalPolicePatrol(const FVector& CrimeLocation)
{
	// Natural road network spawn zone: At least 55m away from player to prevent unnatural popping
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	const FVector PlayerLoc = PlayerPawn ? PlayerPawn->GetActorLocation() : CrimeLocation;

	// Pick random angle outside player sight
	const float RandomAngleRad = FMath::FRandRange(0.0f, 2.0f * PI);
	const float SpawnRadius = FMath::FRandRange(5500.0f, 7500.0f); // 55m - 75m away

	const FVector NaturalSpawnPos = PlayerLoc + FVector(FMath::Cos(RandomAngleRad) * SpawnRadius, FMath::Sin(RandomAngleRad) * SpawnRadius, 40.0f);

	// In a full editor environment, this spawns ARealWorldVehicleBase (Police Cruiser)
	// and routes AI along navigation mesh to LastKnownPlayerLocation.
}
