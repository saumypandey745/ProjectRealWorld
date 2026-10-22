// ============================================================================
// RealWorldMissionCarChase.h
// Project RealWorld — GTA5-class Story Mission: "Repossession & Police Pursuit"
// Full interactive mission actor with stages, police ambush, and rewards
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealWorldMissionCarChase.generated.h"

class ARealWorldCharacter;
class ARealWorldVehicleBase;
class USoundBase;

UENUM(BlueprintType)
enum class ECarChaseStage : uint8
{
    NotStarted          UMETA(DisplayName = "Not Started"),
    BriefingCall        UMETA(DisplayName = "Briefing Call"),
    GoToDepot           UMETA(DisplayName = "Go To Depot"),
    EnterVehicle        UMETA(DisplayName = "Enter Vehicle"),
    PoliceAmbush        UMETA(DisplayName = "Police Ambush & Pursuit"),
    DeliverToHarbor     UMETA(DisplayName = "Deliver To Harbor"),
    MissionPassed       UMETA(DisplayName = "Mission Passed"),
    MissionFailed       UMETA(DisplayName = "Mission Failed")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStageChanged, ECarChaseStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCarChaseFinished, bool, bSuccess, float, RewardAmount);

UCLASS(BlueprintType, Blueprintable)
class PROJECTREALWORLD_API ARealWorldMissionCarChase : public AActor
{
    GENERATED_BODY()

public:
    ARealWorldMissionCarChase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Mission Lifecycle ───────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Mission|CarChase")
    void StartMission();

    UFUNCTION(BlueprintCallable, Category = "Mission|CarChase")
    void AdvanceStage(ECarChaseStage NextStage);

    UFUNCTION(BlueprintCallable, Category = "Mission|CarChase")
    void FailMission(const FText& FailReason);

    UFUNCTION(BlueprintCallable, Category = "Mission|CarChase")
    void CompleteMission();

    // ─── Current State ───────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|State")
    ECarChaseStage CurrentStage = ECarChaseStage::NotStarted;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|State")
    float StageTimer = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|State")
    float TargetVehicleHealthPercent = 100.0f;

    // ─── Mission Locations ───────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Waypoints")
    FVector DepotLocation = FVector(2500.0f, 1200.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Waypoints")
    FVector HarborDropoffLocation = FVector(-7500.0f, 9200.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Waypoints")
    float DropoffAcceptanceRadius = 450.0f;

    // ─── Rewards & Balancing ─────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Rewards")
    float CashReward = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Rewards")
    int32 ReputationReward = 75;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Balancing")
    int32 PursuitWantedStars = 2;

    // ─── SFX / Dialogue ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Audio")
    TObjectPtr<USoundBase> MissionPassedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Audio")
    TObjectPtr<USoundBase> MissionFailedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Audio")
    TObjectPtr<USoundBase> PoliceScannerAudio;

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Mission|Events")
    FOnMissionStageChanged OnStageChanged;

    UPROPERTY(BlueprintAssignable, Category = "Mission|Events")
    FOnCarChaseFinished OnMissionFinished;

protected:
    UPROPERTY()
    TObjectPtr<ARealWorldCharacter> PlayerCharacter;

    UPROPERTY()
    TObjectPtr<ARealWorldVehicleBase> TargetCar;

    void TriggerPolicePursuit();
    void CheckDropoffProximity();
    void PlayMissionSound(USoundBase* Sound);
};
