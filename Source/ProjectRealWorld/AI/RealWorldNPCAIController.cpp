#include "AI/RealWorldNPCAIController.h"
#include "AI/RealWorldNPCCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Subsystems/PoliceWantedSubsystem.h"
#include "Engine/World.h"

ARealWorldNPCAIController::ARealWorldNPCAIController()
{
	SetupPerception();
}

void ARealWorldNPCAIController::SetupPerception()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	// Sight Config
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2500.0f;
	SightConfig->LoseSightRadius = 3000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 75.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// Hearing Config
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 3500.0f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->ConfigureSense(*HearingConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

	AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ARealWorldNPCAIController::OnPerceptionUpdated);
}

void ARealWorldNPCAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ARealWorldNPCAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	ARealWorldNPCCharacter* NPC = Cast<ARealWorldNPCCharacter>(GetPawn());
	if (!NPC || NPC->CurrentState == ENPCState::DEAD) return;

	for (AActor* Actor : UpdatedActors)
	{
		FActorPerceptionBlueprintInfo Info;
		AIPerceptionComp->GetActorsPerception(Actor, Info);

		for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
		{
			if (Stimulus.WasSuccessfullySensed())
			{
				// If gunshots or explosions are heard, react to danger
				if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
				{
					NPC->ReactToDanger(Stimulus.StimulusLocation);
					return;
				}
			}
		}
	}
}

void ARealWorldNPCAIController::OnTargetPerceptionInfoUpdated(const FActorPerceptionUpdateInfo& UpdateInfo)
{
	// Process real-time vision line of sight changes
}

void ARealWorldNPCAIController::OnNPCStateChanged(ENPCState NewState)
{
	ARealWorldNPCCharacter* NPC = Cast<ARealWorldNPCCharacter>(GetPawn());
	if (!NPC) return;

	switch (NewState)
	{
	case ENPCState::IDLE:
		StopMovement();
		break;
	case ENPCState::WALKING:
	case ENPCState::WORKING:
	case ENPCState::GOING_HOME:
		// Move towards destination
		break;
	case ENPCState::FLEEING:
		// Sprint away from danger origin
		break;
	case ENPCState::CALLING_POLICE:
		StopMovement();
		// Notify police subsystem
		if (UWorld* World = GetWorld())
		{
			if (UPoliceWantedSubsystem* PoliceSub = World->GetSubsystem<UPoliceWantedSubsystem>())
			{
				PoliceSub->ReportCrime(ECrimeType::DischargingFirearm, NPC->GetActorLocation(), nullptr, false);
			}
		}
		break;
	default:
		break;
	}
}
