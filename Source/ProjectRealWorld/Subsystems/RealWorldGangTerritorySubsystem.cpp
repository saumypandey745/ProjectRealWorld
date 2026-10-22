// ============================================================================
// RealWorldGangTerritorySubsystem.cpp
// Project RealWorld — GTA: San Andreas & GTA V Style Gang Turf Wars
// ============================================================================

#include "Subsystems/RealWorldGangTerritorySubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void URealWorldGangTerritorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    SetupInitialTerritories();
}

void URealWorldGangTerritorySubsystem::Deinitialize()
{
    Territories.Empty();
    Super::Deinitialize();
}

void URealWorldGangTerritorySubsystem::SetupInitialTerritories()
{
    Territories.Empty();

    // 1. Grove Street / Chamberlain Hills (Families Territory)
    FGangTerritory Chamberlain;
    Chamberlain.TerritoryName = TEXT("Chamberlain Hills");
    Chamberlain.CenterLocation = FVector(-20000.0f, -15000.0f, 0.0f);
    Chamberlain.Radius = 7500.0f;
    Chamberlain.ControllingFaction = EGangFaction::Families;
    Chamberlain.ControlPercent = 100.0f;
    Chamberlain.DailyIncome = 1200;
    Territories.Add(Chamberlain);

    // 2. Davis / Forum Drive (Ballas Territory)
    FGangTerritory Davis;
    Davis.TerritoryName = TEXT("Davis (Ballas Turf)");
    Davis.CenterLocation = FVector(-12000.0f, -22000.0f, 0.0f);
    Davis.Radius = 8000.0f;
    Davis.ControllingFaction = EGangFaction::Ballas;
    Davis.ControlPercent = 100.0f;
    Davis.DailyIncome = 1500;
    Territories.Add(Davis);

    // 3. Rancho / Jamestown (Vagos Territory)
    FGangTerritory Rancho;
    Rancho.TerritoryName = TEXT("Rancho Projects (Vagos)");
    Rancho.CenterLocation = FVector(8000.0f, -26000.0f, 0.0f);
    Rancho.Radius = 9000.0f;
    Rancho.ControllingFaction = EGangFaction::Vagos;
    Rancho.ControlPercent = 100.0f;
    Rancho.DailyIncome = 1800;
    Territories.Add(Rancho);
}

void URealWorldGangTerritorySubsystem::CheckPlayerTerritory(const FVector& PlayerLocation)
{
    int32 FoundIndex = -1;
    for (int32 i = 0; i < Territories.Num(); ++i)
    {
        const float DistSq = FVector::DistSquared2D(PlayerLocation, Territories[i].CenterLocation);
        if (DistSq <= (Territories[i].Radius * Territories[i].Radius))
        {
            FoundIndex = i;
            break;
        }
    }

    if (FoundIndex != CurrentTerritoryIndex)
    {
        CurrentTerritoryIndex = FoundIndex;
        if (CurrentTerritoryIndex >= 0 && Territories.IsValidIndex(CurrentTerritoryIndex))
        {
            OnTerritoryEntered.Broadcast(Territories[CurrentTerritoryIndex].TerritoryName,
                                       Territories[CurrentTerritoryIndex].ControllingFaction);
        }
    }
}

bool URealWorldGangTerritorySubsystem::InitiateTerritoryTakeover(int32 TerritoryIndex)
{
    if (!Territories.IsValidIndex(TerritoryIndex)) return false;

    FGangTerritory& Target = Territories[TerritoryIndex];
    if (Target.ControllingFaction == EGangFaction::Families || Target.bUnderAttack)
    {
        return false;
    }

    Target.bUnderAttack = true;
    Target.CurrentWave = 1;
    SpawnWaveEnemies(TerritoryIndex, 1);

    OnTerritoryWaveStarted.Broadcast(Target.TerritoryName, 1, 3);
    return true;
}

void URealWorldGangTerritorySubsystem::SpawnWaveEnemies(int32 TerritoryIndex, int32 Wave)
{
    WaveEnemiesRemaining = Wave * 4; // Wave 1 = 4 enemies, Wave 2 = 8, Wave 3 = 12
}

void URealWorldGangTerritorySubsystem::OnEnemyGangMemberKilled(int32 TerritoryIndex)
{
    if (!Territories.IsValidIndex(TerritoryIndex)) return;

    FGangTerritory& Target = Territories[TerritoryIndex];
    if (!Target.bUnderAttack) return;

    WaveEnemiesRemaining = FMath::Max(0, WaveEnemiesRemaining - 1);
    PlayerRespect = FMath::Clamp(PlayerRespect + 2, 0, 100);

    // Wave completed
    if (WaveEnemiesRemaining <= 0)
    {
        if (Target.CurrentWave < 3)
        {
            Target.CurrentWave++;
            SpawnWaveEnemies(TerritoryIndex, Target.CurrentWave);
            OnTerritoryWaveStarted.Broadcast(Target.TerritoryName, Target.CurrentWave, 3);
        }
        else
        {
            // Victory: Territory Captured for The Families
            Target.bUnderAttack = false;
            Target.ControllingFaction = EGangFaction::Families;
            Target.ControlPercent = 100.0f;
            PlayerRespect = FMath::Clamp(PlayerRespect + 20, 0, 100);

            OnTerritoryCaptured.Broadcast(Target.TerritoryName, EGangFaction::Families);
        }
    }
}

void URealWorldGangTerritorySubsystem::CollectTerritoryIncome()
{
    int32 TotalIncome = 0;
    for (const FGangTerritory& T : Territories)
    {
        if (T.ControllingFaction == EGangFaction::Families)
        {
            TotalIncome += T.DailyIncome;
        }
    }
}

FGangTerritory URealWorldGangTerritorySubsystem::GetTerritory(int32 Index) const
{
    if (Territories.IsValidIndex(Index))
    {
        return Territories[Index];
    }
    return FGangTerritory();
}
