// ============================================================================
// RealWorldGangTerritorySubsystem.h
// Project RealWorld — GTA: San Andreas & GTA V Style Gang Turf Wars
// Features: Dynamic turf borders, Ballas vs Families vs Vagos,
//           wave-based territory capture, street respect, daily cash income
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RealWorldGangTerritorySubsystem.generated.h"

class ARealWorldCharacter;

UENUM(BlueprintType)
enum class EGangFaction : uint8
{
    None        UMETA(DisplayName = "Neutral / Civilian"),
    Families    UMETA(DisplayName = "The Families (Green)"),
    Ballas      UMETA(DisplayName = "Ballas (Purple)"),
    Vagos       UMETA(DisplayName = "Los Santos Vagos (Yellow)"),
};

USTRUCT(BlueprintType)
struct FGangTerritory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TerritoryName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 6000.0f; // Territory boundary radius (cm)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGangFaction ControllingFaction = EGangFaction::Ballas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ControlPercent = 100.0f; // 0 - 100%

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DailyIncome = 500;       // Cash paid to player if owned by Families

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bUnderAttack = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 CurrentWave = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerritoryEntered, const FString&, TerritoryName, EGangFaction, ControllingFaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTerritoryWaveStarted, const FString&, TerritoryName, int32, WaveNumber, int32, TotalWaves);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerritoryCaptured, const FString&, TerritoryName, EGangFaction, NewFaction);

UCLASS()
class PROJECTREALWORLD_API URealWorldGangTerritorySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "GangWars|Actions")
    void CheckPlayerTerritory(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "GangWars|Actions")
    bool InitiateTerritoryTakeover(int32 TerritoryIndex);

    UFUNCTION(BlueprintCallable, Category = "GangWars|Actions")
    void OnEnemyGangMemberKilled(int32 TerritoryIndex);

    UFUNCTION(BlueprintCallable, Category = "GangWars|Actions")
    void CollectTerritoryIncome();

    // ─── Queries ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "GangWars|State")
    int32 GetTerritoryCount() const { return Territories.Num(); }

    UFUNCTION(BlueprintPure, Category = "GangWars|State")
    FGangTerritory GetTerritory(int32 Index) const;

    UFUNCTION(BlueprintPure, Category = "GangWars|State")
    int32 GetCurrentTerritoryIndex() const { return CurrentTerritoryIndex; }

    UFUNCTION(BlueprintPure, Category = "GangWars|State")
    int32 GetPlayerRespect() const { return PlayerRespect; }

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "GangWars|Events")
    FOnTerritoryEntered OnTerritoryEntered;

    UPROPERTY(BlueprintAssignable, Category = "GangWars|Events")
    FOnTerritoryWaveStarted OnTerritoryWaveStarted;

    UPROPERTY(BlueprintAssignable, Category = "GangWars|Events")
    FOnTerritoryCaptured OnTerritoryCaptured;

private:
    UPROPERTY()
    TArray<FGangTerritory> Territories;

    int32 CurrentTerritoryIndex = -1;
    int32 PlayerRespect = 50; // 0 - 100
    int32 WaveEnemiesRemaining = 0;

    void SetupInitialTerritories();
    void SpawnWaveEnemies(int32 TerritoryIndex, int32 Wave);
};
