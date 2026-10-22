// ============================================================================
// RealWorldMissionHeist.cpp
// Project RealWorld — GTA5-class Major Story Mission: "The Harbor Vault Heist"
// ============================================================================

#include "Subsystems/RealWorldMissionHeist.h"
#include "Character/RealWorldCharacter.h"
#include "Subsystems/PoliceWantedSubsystem.h"
#include "Kismet/GameplayStatics.h"

ARealWorldMissionHeist::ARealWorldMissionHeist()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentStage = EHeistStage::NotStarted;
}

void ARealWorldMissionHeist::BeginPlay()
{
    Super::BeginPlay();

    PlayerCharacter = Cast<ARealWorldCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void ARealWorldMissionHeist::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentStage == EHeistStage::NotStarted ||
        CurrentStage == EHeistStage::HeistPassed ||
        CurrentStage == EHeistStage::HeistFailed)
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
        case EHeistStage::Briefing:
            if (StageTimer >= 4.0f)
            {
                AdvanceHeistStage(EHeistStage::InfiltrateTerminal);
            }
            break;

        case EHeistStage::InfiltrateTerminal:
            if (FVector::Dist(PlayerLoc, TerminalLocation) <= 450.0f)
            {
                AdvanceHeistStage(EHeistStage::DisableCCTV);
            }
            break;

        case EHeistStage::DisableCCTV:
            if (FVector::Dist(PlayerLoc, TransformerLocation) <= 250.0f)
            {
                // Transformer disabled
                AdvanceHeistStage(EHeistStage::VaultDrillBreach);
            }
            break;

        case EHeistStage::VaultDrillBreach:
            // Drill progress over time
            DrillProgressPercent = FMath::Clamp((StageTimer / DrillDuration) * 100.0f, 0.0f, 100.0f);
            if (DrillProgressPercent >= 100.0f)
            {
                PlayHeistSound(VaultOpenAlarmSound);
                AdvanceHeistStage(EHeistStage::LootVaultCash);
            }
            break;

        case EHeistStage::LootVaultCash:
            // Vault cash looted after 3 seconds in vault
            if (StageTimer >= 3.0f)
            {
                AdvanceHeistStage(EHeistStage::SWATEscape);
            }
            break;

        case EHeistStage::SWATEscape:
            // Check if player has reached the safehouse
            if (FVector::Dist(PlayerLoc, SafehouseLocation) <= 500.0f)
            {
                CompleteHeist();
            }
            break;

        default:
            break;
    }
}

void ARealWorldMissionHeist::StartHeist()
{
    StageTimer = 0.0f;
    DrillProgressPercent = 0.0f;
    CurrentStage = EHeistStage::Briefing;

    OnHeistStageChanged.Broadcast(CurrentStage);
}

void ARealWorldMissionHeist::AdvanceHeistStage(EHeistStage NextStage)
{
    CurrentStage = NextStage;
    StageTimer = 0.0f;

    if (NextStage == EHeistStage::VaultDrillBreach)
    {
        PlayHeistSound(DrillHummingSound);
    }
    else if (NextStage == EHeistStage::SWATEscape)
    {
        TriggerSWATResponse();
    }

    OnHeistStageChanged.Broadcast(CurrentStage);
}

void ARealWorldMissionHeist::FailHeist(const FText& Reason)
{
    CurrentStage = EHeistStage::HeistFailed;
    OnHeistStageChanged.Broadcast(CurrentStage);
    OnHeistFinished.Broadcast(false, 0.0f, 0);
}

void ARealWorldMissionHeist::CompleteHeist()
{
    CurrentStage = EHeistStage::HeistPassed;

    // Clear Wanted Level on successful heist escape
    if (UWorld* World = GetWorld())
    {
        if (UPoliceWantedSubsystem* WantedSub = World->GetSubsystem<UPoliceWantedSubsystem>())
        {
            WantedSub->WantedLevel = 0;
            WantedSub->CurrentPursuitState = EPursuitState::Calm;
        }
    }

    PlayHeistSound(HeistPassedMusic);
    OnHeistStageChanged.Broadcast(CurrentStage);
    OnHeistFinished.Broadcast(true, HeistLootTotal, HeistReputation);
}

void ARealWorldMissionHeist::TriggerSWATResponse()
{
    if (UWorld* World = GetWorld())
    {
        if (UPoliceWantedSubsystem* WantedSub = World->GetSubsystem<UPoliceWantedSubsystem>())
        {
            WantedSub->WantedLevel = EscapeWantedStars; // 4-Star SWAT response
            WantedSub->CurrentPursuitState = EPursuitState::ActivePursuit;
            WantedSub->ReportCrime(
                ECrimeType::Homicide,
                VaultLocation,
                PlayerCharacter,
                true
            );
        }
    }
}

void ARealWorldMissionHeist::PlayHeistSound(USoundBase* Sound)
{
    if (Sound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Sound);
    }
}
