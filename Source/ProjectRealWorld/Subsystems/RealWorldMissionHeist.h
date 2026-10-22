// ============================================================================
// RealWorldMissionHeist.h
// Project RealWorld — GTA5-class Major Story Mission: "The Harbor Vault Heist"
// 6-stage tactical heist: CCTV breach, thermite drill, $50,000 loot, 4-Star SWAT escape
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealWorldMissionHeist.generated.h"

class ARealWorldCharacter;
class USoundBase;

UENUM(BlueprintType)
enum class EHeistStage : uint8
{
    NotStarted          UMETA(DisplayName = "Not Started"),
    Briefing            UMETA(DisplayName = "Heist Briefing"),
    InfiltrateTerminal  UMETA(DisplayName = "Infiltrate Security Terminal"),
    DisableCCTV         UMETA(DisplayName = "Disable CCTV Transformer"),
    VaultDrillBreach    UMETA(DisplayName = "Thermite Drill & Defense"),
    LootVaultCash       UMETA(DisplayName = "Loot Vault ($50,000)"),
    SWATEscape          UMETA(DisplayName = "4-Star SWAT Escape"),
    SafehouseArrival    UMETA(DisplayName = "Reach Safehouse"),
    HeistPassed         UMETA(DisplayName = "Heist Passed"),
    HeistFailed         UMETA(DisplayName = "Heist Failed")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeistStageChanged, EHeistStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHeistFinished, bool, bSuccess, float, LootAmount, int32, ReputationEarned);

UCLASS(BlueprintType, Blueprintable)
class PROJECTREALWORLD_API ARealWorldMissionHeist : public AActor
{
    GENERATED_BODY()

public:
    ARealWorldMissionHeist();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Lifecycle ───────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Heist")
    void StartHeist();

    UFUNCTION(BlueprintCallable, Category = "Heist")
    void AdvanceHeistStage(EHeistStage NextStage);

    UFUNCTION(BlueprintCallable, Category = "Heist")
    void FailHeist(const FText& Reason);

    UFUNCTION(BlueprintCallable, Category = "Heist")
    void CompleteHeist();

    // ─── State ───────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heist|State")
    EHeistStage CurrentStage = EHeistStage::NotStarted;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heist|State")
    float StageTimer = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heist|State")
    float DrillProgressPercent = 0.0f; // 0 to 100%

    // ─── Locations ───────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heist|Waypoints")
    FVector TerminalLocation = FVector(-6200.0f, 8500.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heist|Waypoints")
    FVector TransformerLocation = FVector(-6350.0f, 8620.0f, 20.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heist|Waypoints")
    FVector VaultLocation = FVector(-6800.0f, 9100.0f, -50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heist|Waypoints")
    FVector SafehouseLocation = FVector(4200.0f, -3800.0f, 0.0f);

    // ─── Economy & Balancing ─────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heist|Rewards")
    float HeistLootTotal = 50000.0f; // $50,000 cash

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heist|Rewards")
    int32 HeistReputation = 250;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heist|Balancing")
    int32 EscapeWantedStars = 4; // 4-Star SWAT response

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heist|Balancing")
    float DrillDuration = 25.0f; // Seconds to drill vault

    // ─── Audio ───────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heist|Audio")
    TObjectPtr<USoundBase> DrillHummingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heist|Audio")
    TObjectPtr<USoundBase> VaultOpenAlarmSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heist|Audio")
    TObjectPtr<USoundBase> HeistPassedMusic;

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Heist|Events")
    FOnHeistStageChanged OnHeistStageChanged;

    UPROPERTY(BlueprintAssignable, Category = "Heist|Events")
    FOnHeistFinished OnHeistFinished;

protected:
    UPROPERTY()
    TObjectPtr<ARealWorldCharacter> PlayerCharacter;

    void TriggerSWATResponse();
    void PlayHeistSound(USoundBase* Sound);
};
