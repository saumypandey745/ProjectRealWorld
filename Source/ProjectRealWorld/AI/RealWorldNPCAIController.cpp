// ============================================================================
// RealWorldNPCAIController.cpp
// Project RealWorld — NPC AI Controller
// Full navigation: WALKING, FLEEING, WORKING, SHOPPING, GOING_HOME states
// Perception: Sight + Hearing reactions, police reporting, panic cascades
// ============================================================================

#include "AI/RealWorldNPCAIController.h"
#include "AI/RealWorldNPCCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Subsystems/PoliceWantedSubsystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ARealWorldNPCAIController::ARealWorldNPCAIController()
{
	SetupPerception();
	bAttachToPawn = true;
}

void ARealWorldNPCAIController::SetupPerception()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	// Sight Config — standard GTA pedestrian sight cone
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2500.0f;
	SightConfig->LoseSightRadius = 3000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 75.0f;
	SightConfig->SetMaxAge(5.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals  = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// Hearing Config — hear gunshots / engines / shouts
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 3500.0f;
	HearingConfig->SetMaxAge(8.0f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies   = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals  = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->ConfigureSense(*HearingConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

	AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ARealWorldNPCAIController::OnPerceptionUpdated);
}

// ─────────────────────────────────────────────────────────────────────────────
// POSSESS / UNPOSSESS
// ─────────────────────────────────────────────────────────────────────────────
void ARealWorldNPCAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ARealWorldNPCCharacter* NPC = Cast<ARealWorldNPCCharacter>(InPawn);
	if (NPC)
	{
		// Wire state-change callback from character so AI reacts to every state update
		NPC->OnNPCStateChangedDelegate.AddDynamic(this, &ARealWorldNPCAIController::OnNPCStateChanged);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// STATE-DRIVEN NAVIGATION (called whenever NPC state changes)
// ─────────────────────────────────────────────────────────────────────────────
void ARealWorldNPCAIController::OnNPCStateChanged(ENPCState NewState)
{
	ARealWorldNPCCharacter* NPC = Cast<ARealWorldNPCCharacter>(GetPawn());
	if (!NPC) return;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

	switch (NewState)
	{
	// ── Idle / Sleeping: stop all movement ───────────────────────────────────
	case ENPCState::IDLE:
		StopMovement();
		NPC->GetCharacterMovement()->MaxWalkSpeed = 0.0f;
		break;

	// ── Walking / Socializing / Eating: wander on navmesh ───────────────────
	case ENPCState::WALKING:
	case ENPCState::SOCIALIZING:
	case ENPCState::EATING:
	{
		NPC->GetCharacterMovement()->MaxWalkSpeed = 180.0f;
		MoveToRandomNavMeshPoint(NPC, 2000.0f);
		break;
	}

	// ── Working: navigate to schedule destination ─────────────────────────────
	case ENPCState::WORKING:
	{
		NPC->GetCharacterMovement()->MaxWalkSpeed = 200.0f;
		if (!NPC->CurrentScheduleDestination.IsZero())
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, NPC->CurrentScheduleDestination);
		}
		else
		{
			MoveToRandomNavMeshPoint(NPC, 3000.0f);
		}
		break;
	}

	// ── Shopping: navigate to schedule destination (slower, browsing) ─────────
	case ENPCState::SHOPPING:
	{
		NPC->GetCharacterMovement()->MaxWalkSpeed = 140.0f;
		if (!NPC->CurrentScheduleDestination.IsZero())
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, NPC->CurrentScheduleDestination);
		}
		break;
	}

	// ── Going Home: fast walk to home destination ─────────────────────────────
	case ENPCState::GOING_HOME:
	{
		NPC->GetCharacterMovement()->MaxWalkSpeed = 220.0f;
		if (!NPC->HomeLocation.IsZero())
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, NPC->HomeLocation);
		}
		else
		{
			MoveToRandomNavMeshPoint(NPC, 5000.0f);
		}
		break;
	}

	// ── Fleeing: sprint away from danger ─────────────────────────────────────
	case ENPCState::FLEEING:
	{
		NPC->GetCharacterMovement()->MaxWalkSpeed = 520.0f; // panic sprint
		FleeFromDanger(NPC);
		break;
	}

	// ── Calling Police: stop + report crime ───────────────────────────────────
	case ENPCState::CALLING_POLICE:
	{
		StopMovement();
		NPC->GetCharacterMovement()->MaxWalkSpeed = 0.0f;

		if (UWorld* World = GetWorld())
		{
			if (UPoliceWantedSubsystem* PoliceSub = World->GetSubsystem<UPoliceWantedSubsystem>())
			{
				// Report player's last known position as crime location
				APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
				const FVector CrimeLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : NPC->GetActorLocation();
				PoliceSub->ReportCrime(ECrimeType::DischargingFirearm, CrimeLocation, nullptr, false);
			}
		}
		break;
	}

	// ── Dead ─────────────────────────────────────────────────────────────────
	case ENPCState::DEAD:
		StopMovement();
		SetActorTickEnabled(false);
		break;

	default:
		break;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// PERCEPTION CALLBACKS
// ─────────────────────────────────────────────────────────────────────────────
void ARealWorldNPCAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	ARealWorldNPCCharacter* NPC = Cast<ARealWorldNPCCharacter>(GetPawn());
	if (!NPC || NPC->CurrentState == ENPCState::DEAD) return;

	// Already fleeing — don't re-trigger
	if (NPC->CurrentState == ENPCState::FLEEING) return;

	for (AActor* Actor : UpdatedActors)
	{
		FActorPerceptionBlueprintInfo Info;
		AIPerceptionComp->GetActorsPerception(Actor, Info);

		for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
		{
			if (!Stimulus.WasSuccessfullySensed()) continue;

			// ── HEARING: gunshot / explosion ─────────────────────────────────
			if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
			{
				// React to danger — flee or call police based on archetype
				NPC->ReactToDanger(Stimulus.StimulusLocation);
				return;
			}

			// ── SIGHT: player or threatening actor ───────────────────────────
			if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
			{
				// Check if we directly see the player doing something threatening
				APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
				if (Actor == PlayerPawn)
				{
					// Civilians react to player wielding a weapon
					if (NPC->Archetype == ENPCArchetype::CivilianMale ||
						NPC->Archetype == ENPCArchetype::CivilianFemale)
					{
						NPC->ReactToDanger(Actor->GetActorLocation());
					}
				}
				return;
			}
		}
	}
}

void ARealWorldNPCAIController::OnTargetPerceptionInfoUpdated(const FActorPerceptionUpdateInfo& UpdateInfo)
{
	// Fine-grained per-target LoS changes handled via OnPerceptionUpdated
}

// ─────────────────────────────────────────────────────────────────────────────
// NAVIGATION HELPERS
// ─────────────────────────────────────────────────────────────────────────────
void ARealWorldNPCAIController::MoveToRandomNavMeshPoint(ARealWorldNPCCharacter* NPC, float Radius)
{
	if (!NPC) return;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return;

	FNavLocation RandomLocation;
	const bool bFound = NavSys->GetRandomReachablePointInRadius(
		NPC->GetActorLocation(), Radius, RandomLocation
	);

	if (bFound)
	{
		// Store as current wander target and move
		MoveToLocation(RandomLocation.Location, 50.0f, true, true, false, true);
	}
}

void ARealWorldNPCAIController::FleeFromDanger(ARealWorldNPCCharacter* NPC)
{
	if (!NPC) return;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return;

	// Find direction AWAY from danger origin
	const FVector DangerOrigin  = NPC->LastDangerLocation;
	const FVector FleeDirection = (NPC->GetActorLocation() - DangerOrigin).GetSafeNormal();
	const FVector FleeTarget    = NPC->GetActorLocation() + FleeDirection * 3000.0f;

	// Project onto navmesh
	FNavLocation NavFleeTarget;
	const bool bFound = NavSys->ProjectPointToNavigation(
		FleeTarget, NavFleeTarget, FVector(500.0f, 500.0f, 500.0f)
	);

	if (bFound)
	{
		MoveToLocation(NavFleeTarget.Location, 100.0f, true, true, false, true);
	}
	else
	{
		// Fallback: random reachable point
		MoveToRandomNavMeshPoint(NPC, 4000.0f);
	}
}
