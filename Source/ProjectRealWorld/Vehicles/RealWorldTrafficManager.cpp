#include "Vehicles/RealWorldTrafficManager.h"
#include "Vehicles/RealWorldVehicleBase.h"
#include "Character/RealWorldCharacter.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"
#include "ChaosWheeledVehicleMovementComponent.h"

ARealWorldTrafficManager::ARealWorldTrafficManager()
{
	PrimaryActorTick.bCanEverTick = true;

	RoadSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RoadSpline"));
	SetRootComponent(RoadSpline);

	NorthSouthSignal = ETrafficLightState::Green;
	EastWestSignal = ETrafficLightState::Red;
	GreenPhaseDuration = 24.0f;
	YellowPhaseDuration = 3.5f;

	DensitySetting = ETrafficDensity::Medium;
	MinSpawnDistance = 3500.0f;
	MaxSpawnDistance = 9000.0f;
	DespawnDistance = 14000.0f;

	SignalPhaseTimer = 0.0f;
	bNorthSouthIsActiveGreen = true;
	SpawnCheckTimer = 0.0f;
}

void ARealWorldTrafficManager::BeginPlay()
{
	Super::BeginPlay();
}

void ARealWorldTrafficManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTrafficSignals(DeltaTime);
	ManageVehicleSpawningAndDespawning(DeltaTime);
	SimulateTrafficAgents(DeltaTime);
}

void ARealWorldTrafficManager::UpdateTrafficSignals(float DeltaTime)
{
	SignalPhaseTimer += DeltaTime;

	if (bNorthSouthIsActiveGreen)
	{
		// North-South cycle
		if (SignalPhaseTimer < GreenPhaseDuration)
		{
			NorthSouthSignal = ETrafficLightState::Green;
			EastWestSignal = ETrafficLightState::Red;
		}
		else if (SignalPhaseTimer < (GreenPhaseDuration + YellowPhaseDuration))
		{
			NorthSouthSignal = ETrafficLightState::Yellow;
			EastWestSignal = ETrafficLightState::Red;
		}
		else
		{
			// Switch to East-West
			bNorthSouthIsActiveGreen = false;
			SignalPhaseTimer = 0.0f;
			NorthSouthSignal = ETrafficLightState::Red;
			EastWestSignal = ETrafficLightState::Green;
		}
	}
	else
	{
		// East-West cycle
		if (SignalPhaseTimer < GreenPhaseDuration)
		{
			EastWestSignal = ETrafficLightState::Green;
			NorthSouthSignal = ETrafficLightState::Red;
		}
		else if (SignalPhaseTimer < (GreenPhaseDuration + YellowPhaseDuration))
		{
			EastWestSignal = ETrafficLightState::Yellow;
			NorthSouthSignal = ETrafficLightState::Red;
		}
		else
		{
			// Switch to North-South
			bNorthSouthIsActiveGreen = true;
			SignalPhaseTimer = 0.0f;
			EastWestSignal = ETrafficLightState::Red;
			NorthSouthSignal = ETrafficLightState::Green;
		}
	}
}

int32 ARealWorldTrafficManager::GetMaxActiveVehiclesForDensity() const
{
	switch (DensitySetting)
	{
	case ETrafficDensity::Off: return 0;
	case ETrafficDensity::Low: return 6;
	case ETrafficDensity::Medium: return 14;
	case ETrafficDensity::High: return 24;
	case ETrafficDensity::RushHour: return 36;
	default: return 12;
	}
}

float ARealWorldTrafficManager::ComputeSafeFollowingDistance(float CurrentSpeedKmH) const
{
	// Believable following distance: base 4 meters plus 1.2m per 10 km/h
	return 400.0f + (FMath::Max(0.0f, CurrentSpeedKmH) * 12.0f);
}

bool ARealWorldTrafficManager::CheckForwardObstacle(FVector Origin, FVector ForwardVector, float LookaheadDistance) const
{
	const FVector TraceEnd = Origin + (ForwardVector * LookaheadDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult HitResult;
	return GetWorld()->LineTraceSingleByChannel(HitResult, Origin, TraceEnd, ECC_Vehicle, QueryParams);
}

bool ARealWorldTrafficManager::CheckPedestrianCrossingAhead(FVector Origin, FVector ForwardVector, float LookaheadDistance) const
{
	const FVector TraceEnd = Origin + (ForwardVector * LookaheadDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult HitResult;
	// Pawn channel detects crossing pedestrians
	return GetWorld()->SweepSingleByChannel(
		HitResult,
		Origin,
		TraceEnd,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(120.0f),
		QueryParams
	);
}

void ARealWorldTrafficManager::NotifyEmergencyVehicleInArea(FVector EmergencyLocation, float SirenAudibleRadius)
{
	for (FTrafficAgent& Agent : ActiveAgents)
	{
		if (Agent.Vehicle.IsValid())
		{
			const float Distance = FVector::Dist(Agent.Vehicle->GetActorLocation(), EmergencyLocation);
			if (Distance < SirenAudibleRadius)
			{
				Agent.bYieldingToEmergency = true;
			}
		}
	}
}

void ARealWorldTrafficManager::UpdateAgentLOD(FTrafficAgent& Agent, const FVector& PlayerLocation)
{
	if (!Agent.Vehicle.IsValid()) return;

	const float Distance = FVector::Dist(Agent.Vehicle->GetActorLocation(), PlayerLocation);

	if (Distance < 3000.0f) // < 30m
	{
		Agent.LOD = ETrafficVehicleLOD::Near;
	}
	else if (Distance < 8000.0f) // 30m - 80m
	{
		Agent.LOD = ETrafficVehicleLOD::Medium;
	}
	else if (Distance < 14000.0f) // 80m - 140m
	{
		Agent.LOD = ETrafficVehicleLOD::Far;
	}
	else
	{
		Agent.LOD = ETrafficVehicleLOD::VeryFar;
	}
}

void ARealWorldTrafficManager::ManageVehicleSpawningAndDespawning(float DeltaTime)
{
	SpawnCheckTimer += DeltaTime;
	if (SpawnCheckTimer < 1.0f) return;
	SpawnCheckTimer = 0.0f;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn || !RoadSpline) return;

	const FVector PlayerLoc = PlayerPawn->GetActorLocation();

	// 1. Despawn vehicles outside DespawnDistance or invalid
	for (int32 i = ActiveAgents.Num() - 1; i >= 0; --i)
	{
		FTrafficAgent& Agent = ActiveAgents[i];
		if (!Agent.Vehicle.IsValid())
		{
			ActiveAgents.RemoveAt(i);
			continue;
		}

		const float DistToPlayer = FVector::Dist(Agent.Vehicle->GetActorLocation(), PlayerLoc);
		if (DistToPlayer > DespawnDistance)
		{
			Agent.Vehicle->Destroy();
			ActiveAgents.RemoveAt(i);
		}
	}

	// 2. Spawn new vehicles if below target density
	const int32 MaxVehicles = GetMaxActiveVehiclesForDensity();
	if (ActiveAgents.Num() < MaxVehicles && CivilianVehicleClass)
	{
		const float SplineLength = RoadSpline->GetSplineLength();
		if (SplineLength < 500.0f) return;

		// Find a spawn distance along spline within [MinSpawnDistance, MaxSpawnDistance]
		const float RandomDistanceAlongSpline = FMath::FRandRange(0.0f, SplineLength);
		const FVector CandidatePos = RoadSpline->GetLocationAtDistanceAlongSpline(RandomDistanceAlongSpline, ESplineCoordinateSpace::World);
		const float DistToPlayer = FVector::Dist(CandidatePos, PlayerLoc);

		if (DistToPlayer >= MinSpawnDistance && DistToPlayer <= MaxSpawnDistance)
		{
			// Check clearance
			bool bClear = true;
			for (const FTrafficAgent& Existing : ActiveAgents)
			{
				if (Existing.Vehicle.IsValid() && FVector::Dist(Existing.Vehicle->GetActorLocation(), CandidatePos) < 1200.0f)
				{
					bClear = false;
					break;
				}
			}

			if (bClear)
			{
				const FRotator SpawnRot = RoadSpline->GetRotationAtDistanceAlongSpline(RandomDistanceAlongSpline, ESplineCoordinateSpace::World);
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				ARealWorldVehicleBase* NewVehicle = GetWorld()->SpawnActor<ARealWorldVehicleBase>(CivilianVehicleClass, CandidatePos + FVector(0,0,50), SpawnRot, SpawnParams);
				if (NewVehicle)
				{
					FTrafficAgent NewAgent;
					NewAgent.Vehicle = NewVehicle;
					NewAgent.DistanceAlongSpline = RandomDistanceAlongSpline;
					NewAgent.TargetSpeedKmH = FMath::FRandRange(35.0f, 50.0f);
					NewAgent.CurrentSpeedKmH = NewAgent.TargetSpeedKmH;
					NewAgent.LOD = ETrafficVehicleLOD::Far;
					ActiveAgents.Add(NewAgent);
				}
			}
		}
	}
}

void ARealWorldTrafficManager::SimulateTrafficAgents(float DeltaTime)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	const FVector PlayerLoc = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
	const float SplineLength = RoadSpline ? RoadSpline->GetSplineLength() : 0.0f;

	for (FTrafficAgent& Agent : ActiveAgents)
	{
		if (!Agent.Vehicle.IsValid() || SplineLength <= 0.0f) continue;

		UpdateAgentLOD(Agent, PlayerLoc);

		// Throttled LOD updates
		if (Agent.LOD == ETrafficVehicleLOD::Medium)
		{
			Agent.TickAccumulator += DeltaTime;
			if (Agent.TickAccumulator < 0.05f) continue; // 20 Hz
			Agent.TickAccumulator = 0.0f;
		}
		else if (Agent.LOD == ETrafficVehicleLOD::Far)
		{
			Agent.TickAccumulator += DeltaTime;
			if (Agent.TickAccumulator < 0.1f) continue; // 10 Hz
			Agent.TickAccumulator = 0.0f;
		}

		ARealWorldVehicleBase* Veh = Agent.Vehicle.Get();
		const FVector VehLoc = Veh->GetActorLocation();
		const FVector VehForward = Veh->GetActorForwardVector();

		// Check intersection stop lines
		bool bMustStopAtSignal = false;
		for (const FTrafficStopLine& StopLine : StopLines)
		{
			const float DistToStop = FVector::Dist(VehLoc, StopLine.StopLocation);
			if (DistToStop < StopLine.StopToleranceDistance)
			{
				const ETrafficLightState RelevantSignal = StopLine.bGovernedByNorthSouthSignal ? NorthSouthSignal : EastWestSignal;
				if (RelevantSignal == ETrafficLightState::Red || RelevantSignal == ETrafficLightState::Yellow)
				{
					bMustStopAtSignal = true;
					break;
				}
			}
		}

		Agent.bWaitingAtRedLight = bMustStopAtSignal;

		// Check obstacle and pedestrian ahead
		const float SafeFollowDistance = ComputeSafeFollowingDistance(Agent.CurrentSpeedKmH);
		const bool bObstacleAhead = CheckForwardObstacle(VehLoc + FVector(0,0,50), VehForward, SafeFollowDistance);
		const bool bPedestrianAhead = CheckPedestrianCrossingAhead(VehLoc + FVector(0,0,40), VehForward, SafeFollowDistance * 0.8f);

		// Compute desired speed
		float DesiredSpeed = Agent.TargetSpeedKmH;

		if (bMustStopAtSignal || bObstacleAhead || bPedestrianAhead)
		{
			DesiredSpeed = 0.0f;
		}
		else if (Agent.bYieldingToEmergency)
		{
			DesiredSpeed = 15.0f; // Pull over / crawl speed
		}

		// Smooth deceleration / acceleration
		const float AccelRate = (DesiredSpeed < Agent.CurrentSpeedKmH) ? 25.0f : 12.0f; // Decel faster than accel
		Agent.CurrentSpeedKmH = FMath::FInterpTo(Agent.CurrentSpeedKmH, DesiredSpeed, DeltaTime, AccelRate);

		// Movement execution based on LOD
		if (Agent.LOD == ETrafficVehicleLOD::Near)
		{
			// Full physics simulation
			if (UChaosWheeledVehicleMovementComponent* MoveComp = Cast<UChaosWheeledVehicleMovementComponent>(Veh->GetVehicleMovementComponent()))
			{
				if (Agent.CurrentSpeedKmH > 2.0f)
				{
					MoveComp->SetThrottleInput(0.6f);
					MoveComp->SetBrakeInput(0.0f);
				}
				else
				{
					MoveComp->SetThrottleInput(0.0f);
					MoveComp->SetBrakeInput(1.0f);
				}
			}
		}
		else
		{
			// Medium / Far / VeryFar: Kinematic Spline advancement
			const float StepDistance = (Agent.CurrentSpeedKmH * (100000.0f / 3600.0f)) * DeltaTime; // cm
			Agent.DistanceAlongSpline = FMath::Fmod(Agent.DistanceAlongSpline + StepDistance, SplineLength);

			const FVector NextPos = RoadSpline->GetLocationAtDistanceAlongSpline(Agent.DistanceAlongSpline, ESplineCoordinateSpace::World);
			const FRotator NextRot = RoadSpline->GetRotationAtDistanceAlongSpline(Agent.DistanceAlongSpline, ESplineCoordinateSpace::World);

			Veh->SetActorLocationAndRotation(NextPos, NextRot);
		}
	}
}
