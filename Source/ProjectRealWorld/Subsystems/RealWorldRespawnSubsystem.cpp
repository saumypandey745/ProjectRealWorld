// ============================================================================
// RealWorldRespawnSubsystem.cpp
// Project RealWorld — GTA V-Class "Wasted" and "Busted" Respawn System
// ============================================================================

#include "Subsystems/RealWorldRespawnSubsystem.h"
#include "Character/RealWorldCharacter.h"
#include "Character/RealWorldHealthComponent.h"
#include "Subsystems/PoliceWantedSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void URealWorldRespawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void URealWorldRespawnSubsystem::TriggerWasted(ARealWorldCharacter* Player)
{
    if (!Player || PendingPlayer) return;

    PendingPlayer = Player;
    ActiveReason = ERespawnReason::Wasted;

    // Bullet-time slow motion on death
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGlobalTimeDilation(World, 0.3f);
    }

    if (WastedSound)
    {
        UGameplayStatics::PlaySound2D(this, WastedSound);
    }

    OnPlayerRespawnSequenceStarted.Broadcast(ERespawnReason::Wasted);

    // 4 seconds slow-mo sequence (real time = 4 * 0.3 = 1.2s)
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            RespawnTimerHandle,
            this, &URealWorldRespawnSubsystem::CompleteRespawn,
            3.5f,
            false
        );
    }
}

void URealWorldRespawnSubsystem::TriggerBusted(ARealWorldCharacter* Player)
{
    if (!Player || PendingPlayer) return;

    PendingPlayer = Player;
    ActiveReason = ERespawnReason::Busted;

    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGlobalTimeDilation(World, 0.4f);
    }

    if (BustedSound)
    {
        UGameplayStatics::PlaySound2D(this, BustedSound);
    }

    OnPlayerRespawnSequenceStarted.Broadcast(ERespawnReason::Busted);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            RespawnTimerHandle,
            this, &URealWorldRespawnSubsystem::CompleteRespawn,
            3.0f,
            false
        );
    }
}

void URealWorldRespawnSubsystem::CompleteRespawn()
{
    UWorld* World = GetWorld();
    if (!World || !PendingPlayer) return;

    // Restore normal time dilation
    UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);

    int32 FeeDeducted = 0;

    if (ActiveReason == ERespawnReason::Wasted)
    {
        FeeDeducted = HospitalMedicalBill;
        PendingPlayer->SetActorLocation(PillboxHospitalLocation);

        // Revive character to 100% health
        if (URealWorldHealthComponent* Health = PendingPlayer->FindComponentByClass<URealWorldHealthComponent>())
        {
            Health->Revive(100.0f);
        }
    }
    else // Busted
    {
        FeeDeducted = PoliceBailFee;
        PendingPlayer->SetActorLocation(MissionRowPoliceLocation);

        if (URealWorldHealthComponent* Health = PendingPlayer->FindComponentByClass<URealWorldHealthComponent>())
        {
            Health->Revive(100.0f);
        }

        // Wipe police wanted stars
        if (UPoliceWantedSubsystem* Wanted = World->GetSubsystem<UPoliceWantedSubsystem>())
        {
            Wanted->ClearWantedLevel();
        }
    }

    OnPlayerRespawnCompleted.Broadcast(ActiveReason, FeeDeducted);
    PendingPlayer = nullptr;
}
