// ============================================================================
// RealWorldRespawnSubsystem.h
// Project RealWorld — GTA V-Class "Wasted" and "Busted" Respawn System
// Features: Slow-mo grayscale death sequence, hospital medical bills,
//           police station bail, weapon confiscation, impound lot
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Sound/SoundBase.h"
#include "RealWorldRespawnSubsystem.generated.h"

class ARealWorldCharacter;

UENUM(BlueprintType)
enum class ERespawnReason : uint8
{
    Wasted      UMETA(DisplayName = "Wasted (Death)"),
    Busted      UMETA(DisplayName = "Busted (Arrest)"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerRespawnSequenceStarted, ERespawnReason, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerRespawnCompleted, ERespawnReason, Reason, int32, FeeDeducted);

UCLASS()
class PROJECTREALWORLD_API URealWorldRespawnSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Respawn|Actions")
    void TriggerWasted(ARealWorldCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Respawn|Actions")
    void TriggerBusted(ARealWorldCharacter* Player);

    // ─── Config ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn|Config")
    int32 HospitalMedicalBill = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn|Config")
    int32 PoliceBailFee = 2500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn|Audio")
    TObjectPtr<USoundBase> WastedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn|Audio")
    TObjectPtr<USoundBase> BustedSound;

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Respawn|Events")
    FOnPlayerRespawnSequenceStarted OnPlayerRespawnSequenceStarted;

    UPROPERTY(BlueprintAssignable, Category = "Respawn|Events")
    FOnPlayerRespawnCompleted OnPlayerRespawnCompleted;

private:
    FTimerHandle RespawnTimerHandle;

    UPROPERTY()
    TObjectPtr<ARealWorldCharacter> PendingPlayer;

    ERespawnReason ActiveReason = ERespawnReason::Wasted;

    // Known GTA Respawn Landmarks
    FVector PillboxHospitalLocation = FVector(3500.0f, -6000.0f, 150.0f);
    FVector MissionRowPoliceLocation = FVector(4200.0f, -10000.0f, 150.0f);

    void CompleteRespawn();
};
