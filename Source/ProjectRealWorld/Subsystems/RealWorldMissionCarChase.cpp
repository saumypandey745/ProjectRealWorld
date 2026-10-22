// ============================================================================
// RealWorldMissionCarChase.cpp
// Project RealWorld — GTA5-class Story Mission: "Repossession & Police Pursuit"
// ============================================================================

#include "Subsystems/RealWorldMissionCarChase.h"
#include "Character/RealWorldCharacter.h"
#include "Vehicles/RealWorldVehicleBase.h"
#include "Subsystems/PoliceWantedSubsystem.h"
#include "Kismet/GameplayStatics.h"

ARealWorldMissionCarChase::ARealWorldMissionCarChase()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentStage = ECarChaseStage::NotStarted;
}

void ARealWorldMissionCarChase::BeginPlay()
{
    Super::BeginPlay();

    PlayerCharacter = Cast<ARealWorldCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void ARealWorldMissionCarChase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentStage == ECarChaseStage::NotStarted ||
        CurrentStage == ECarChaseStage::MissionPassed ||
        CurrentStage == ECarChaseStage::MissionFailed)
    {
        return;
    }

    StageTimer += DeltaTime;

    if (!PlayerCharacter)
    {
        PlayerCharacter = Cast<ARealWorldCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
        if (!PlayerCharacter) return;
    }

    const FVector PlayerLoc = PlayerCharacter->GetActorLocation();

    switch (CurrentStage)
    {
        case ECarChaseStage::BriefingCall:
            // Briefing dialogue timer: advance to GoToDepot after 3 seconds
            if (StageTimer >= 3.0f)
            {
                AdvanceStage(ECarChaseStage::GoToDepot);
            }
            break;

        case ECarChaseStage::GoToDepot:
            // Check if player has arrived at the depot
            if (FVector::Dist(PlayerLoc, DepotLocation) <= 800.0f)
            {
                AdvanceStage(ECarChaseStage::EnterVehicle);
            }
            break;

        case ECarChaseStage::EnterVehicle:
            // Detect if player is in a vehicle
            if (PlayerCharacter->GetAttachParentActor() != nullptr &&
                PlayerCharacter->GetAttachParentActor()->IsA<ARealWorldVehicleBase>())
            {
                AdvanceStage(ECarChaseStage::PoliceAmbush);
            }
            break;

        case ECarChaseStage::PoliceAmbush:
            // After initial ambush sirens, objective becomes deliver to harbor
            if (StageTimer >= 4.0f)
            {
                AdvanceStage(ECarChaseStage::DeliverToHarbor);
            }
            break;

        case ECarChaseStage::DeliverToHarbor:
            CheckDropoffProximity();
            break;

        default:
            break;
    }
}

void ARealWorldMissionCarChase::StartMission()
{
    StageTimer = 0.0f;
    CurrentStage = ECarChaseStage::BriefingCall;

    PlayMissionSound(PoliceScannerAudio);
    OnStageChanged.Broadcast(CurrentStage);
}

void ARealWorldMissionCarChase::AdvanceStage(ECarChaseStage NextStage)
{
    CurrentStage = NextStage;
    StageTimer = 0.0f;

    if (NextStage == ECarChaseStage::PoliceAmbush)
    {
        TriggerPolicePursuit();
    }

    OnStageChanged.Broadcast(CurrentStage);
}

void ARealWorldMissionCarChase::FailMission(const FText& FailReason)
{
    CurrentStage = ECarChaseStage::MissionFailed;
    PlayMissionSound(MissionFailedSound);
    OnStageChanged.Broadcast(CurrentStage);
    OnMissionFinished.Broadcast(false, 0.0f);
}

void ARealWorldMissionCarChase::CompleteMission()
{
    CurrentStage = ECarChaseStage::MissionPassed;

    // Reset police wanted stars to 0 on mission pass
    if (UWorld* World = GetWorld())
    {
        if (UPoliceWantedSubsystem* WantedSub = World->GetSubsystem<UPoliceWantedSubsystem>())
        {
            WantedSub->WantedLevel = 0;
            WantedSub->CurrentPursuitState = EPursuitState::Calm;
        }
    }

    PlayMissionSound(MissionPassedSound);
    OnStageChanged.Broadcast(CurrentStage);
    OnMissionFinished.Broadcast(true, CashReward);
}

void ARealWorldMissionCarChase::TriggerPolicePursuit()
{
    if (UWorld* World = GetWorld())
    {
        if (UPoliceWantedSubsystem* WantedSub = World->GetSubsystem<UPoliceWantedSubsystem>())
        {
            WantedSub->ReportCrime(
                ECrimeType::GrandTheftAuto,
                DepotLocation,
                PlayerCharacter,
                true // Direct sight -> triggers active sirens and pursuit
            );
        }
    }

    PlayMissionSound(PoliceScannerAudio);
}

void ARealWorldMissionCarChase::CheckDropoffProximity()
{
    if (!PlayerCharacter) return;

    const FVector PlayerLoc = PlayerCharacter->GetActorLocation();
    const float DistToHarbor = FVector::Dist(PlayerLoc, HarborDropoffLocation);

    if (DistToHarbor <= DropoffAcceptanceRadius)
    {
        CompleteMission();
    }
}

void ARealWorldMissionCarChase::PlayMissionSound(USoundBase* Sound)
{
    if (Sound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Sound);
    }
}
