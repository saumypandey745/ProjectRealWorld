// ============================================================================
// RealWorldMissionDirectorSubsystem.cpp
// Project RealWorld — GTA V-Class Mission Director & Checkpoint Engine
// ============================================================================

#include "Subsystems/RealWorldMissionDirectorSubsystem.h"
#include "Character/RealWorldCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void URealWorldMissionDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bMissionInProgress = false;
}

bool URealWorldMissionDirectorSubsystem::StartMission(const FMissionDefinition& Mission)
{
    if (bMissionInProgress) return false;

    bMissionInProgress = true;
    ActiveMission = Mission;
    CurrentMissionTime = 0.0f;

    // Default starting checkpoint
    LastCheckpoint = FMissionCheckpoint();
    LastCheckpoint.CheckpointIndex = 0;
    LastCheckpoint.ElapsedMissionTime = 0.0f;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MissionTickerHandle,
            this, &URealWorldMissionDirectorSubsystem::TickMission,
            1.0f,
            true
        );
    }

    OnMissionStarted.Broadcast(ActiveMission.Title);
    return true;
}

void URealWorldMissionDirectorSubsystem::TickMission()
{
    if (!bMissionInProgress) return;
    CurrentMissionTime += 1.0f;
}

void URealWorldMissionDirectorSubsystem::SetCheckpoint(int32 CheckpointIndex, const FVector& Location, const FRotator& Rotation)
{
    LastCheckpoint.CheckpointIndex = CheckpointIndex;
    LastCheckpoint.PlayerLocation = Location;
    LastCheckpoint.PlayerRotation = Rotation;
    LastCheckpoint.ElapsedMissionTime = CurrentMissionTime;
}

void URealWorldMissionDirectorSubsystem::FailMission(const FString& Reason)
{
    if (!bMissionInProgress) return;
    bMissionInProgress = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MissionTickerHandle);
    }

    if (MissionFailedSound)
    {
        UGameplayStatics::PlaySound2D(this, MissionFailedSound);
    }

    OnMissionFailed.Broadcast(ActiveMission.Title, Reason);
}

bool URealWorldMissionDirectorSubsystem::RetryLastCheckpoint(ARealWorldCharacter* Player)
{
    if (!Player) return false;

    Player->SetActorLocationAndRotation(LastCheckpoint.PlayerLocation, LastCheckpoint.PlayerRotation);
    CurrentMissionTime = LastCheckpoint.ElapsedMissionTime;
    bMissionInProgress = true;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MissionTickerHandle,
            this, &URealWorldMissionDirectorSubsystem::TickMission,
            1.0f,
            true
        );
    }

    return true;
}

void URealWorldMissionDirectorSubsystem::CompleteMission(int32 HeadshotsScored, float ShootingAccuracy)
{
    if (!bMissionInProgress) return;
    bMissionInProgress = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MissionTickerHandle);
    }

    // Medal Evaluation: Gold requires all 3 criteria, Silver requires 2, Bronze requires completion
    int32 CriteriaPassed = 0;
    if (CurrentMissionTime <= ActiveMission.GoldTimeLimit) CriteriaPassed++;
    if (ShootingAccuracy >= ActiveMission.GoldAccuracyRequired) CriteriaPassed++;
    if (HeadshotsScored >= ActiveMission.GoldHeadshotsRequired) CriteriaPassed++;

    EMissionMedal Medal = EMissionMedal::Bronze;
    if (CriteriaPassed == 3) Medal = EMissionMedal::Gold;
    else if (CriteriaPassed >= 1) Medal = EMissionMedal::Silver;

    if (MissionPassedSound)
    {
        UGameplayStatics::PlaySound2D(this, MissionPassedSound);
    }

    OnMissionPassed.Broadcast(ActiveMission.Title, Medal, ActiveMission.CashReward);
}
