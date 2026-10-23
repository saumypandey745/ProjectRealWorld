// ============================================================================
// RealWorldMissionDirectorSubsystem.h
// Project RealWorld — GTA V-Class Mission Director & Checkpoint Engine
// Features: Dynamic checkpoints, fail reasons, restart from checkpoint,
//           Gold/Silver/Bronze medal grading criteria
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Sound/SoundBase.h"
#include "RealWorldMissionDirectorSubsystem.generated.h"

class ARealWorldCharacter;

UENUM(BlueprintType)
enum class EMissionMedal : uint8
{
    None        UMETA(DisplayName = "Failed / Incomplete"),
    Bronze      UMETA(DisplayName = "Bronze Medal (Passed)"),
    Silver      UMETA(DisplayName = "Silver Medal"),
    Gold        UMETA(DisplayName = "Gold Medal (100% Completion)"),
};

USTRUCT(BlueprintType)
struct FMissionCheckpoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CheckpointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator PlayerRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ElapsedMissionTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FMissionDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CashReward = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GoldTimeLimit = 300.0f; // Seconds to get Gold time medal

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GoldAccuracyRequired = 0.70f; // 70% accuracy

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoldHeadshotsRequired = 10;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDirectorMissionStarted, const FString&, MissionTitle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionFailed, const FString&, MissionTitle, const FString&, FailReason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMissionPassed, const FString&, MissionTitle, EMissionMedal, MedalEarned, int32, CashEarned);

UCLASS()
class PROJECTREALWORLD_API URealWorldMissionDirectorSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "MissionDirector|Actions")
    bool StartMission(const FMissionDefinition& Mission);

    UFUNCTION(BlueprintCallable, Category = "MissionDirector|Actions")
    void SetCheckpoint(int32 CheckpointIndex, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "MissionDirector|Actions")
    void FailMission(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "MissionDirector|Actions")
    bool RetryLastCheckpoint(ARealWorldCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "MissionDirector|Actions")
    void CompleteMission(int32 HeadshotsScored, float ShootingAccuracy);

    // ─── Queries ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "MissionDirector|State")
    bool IsMissionActive() const { return bMissionInProgress; }

    UFUNCTION(BlueprintPure, Category = "MissionDirector|State")
    float GetMissionElapsedTime() const { return CurrentMissionTime; }

    // ─── Audio ───────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MissionDirector|Audio")
    TObjectPtr<USoundBase> MissionPassedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MissionDirector|Audio")
    TObjectPtr<USoundBase> MissionFailedSound;

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "MissionDirector|Events")
    FOnDirectorMissionStarted OnMissionStarted;

    UPROPERTY(BlueprintAssignable, Category = "MissionDirector|Events")
    FOnMissionFailed OnMissionFailed;

    UPROPERTY(BlueprintAssignable, Category = "MissionDirector|Events")
    FOnMissionPassed OnMissionPassed;

private:
    bool bMissionInProgress = false;
    float CurrentMissionTime = 0.0f;
    FMissionDefinition ActiveMission;
    FMissionCheckpoint LastCheckpoint;

    FTimerHandle MissionTickerHandle;
    void TickMission();
};
