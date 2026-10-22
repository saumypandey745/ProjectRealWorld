// ============================================================================
// RealWorldSideJobsSubsystem.h
// Project RealWorld — GTA-Class Side Hustles & Service Jobs
// Features: Taxi Fares & Tips, Paramedic Ambulance Revivals,
//           Police Vigilante Criminal Hunts, Firefighter Blaze Extinguishing
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Sound/SoundBase.h"
#include "RealWorldSideJobsSubsystem.generated.h"

class ARealWorldCharacter;
class AActor;

UENUM(BlueprintType)
enum class ESideJobType : uint8
{
    None        UMETA(DisplayName = "None"),
    Taxi        UMETA(DisplayName = "Downtown Cab Co. (Taxi Driver)"),
    Paramedic   UMETA(DisplayName = "Los Santos Medical (Paramedic)"),
    Vigilante   UMETA(DisplayName = "LSPD Vigilante (Criminal Hunt)"),
    Firefighter UMETA(DisplayName = "LS Fire Dept (Firefighter)"),
};

USTRUCT(BlueprintType)
struct FSideJobSession
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    ESideJobType JobType = ESideJobType::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 ConsecutiveCompleted = 0; // Streak bonus multiplier

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 TotalCashEarned = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector PickupLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector DestinationLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float RemainingTime = 0.0f; // Seconds left

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float CustomerSatisfaction = 1.0f; // 1.0 = 100% tip, decreases with hard crashes

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bHasPassenger = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSideJobStarted, ESideJobType, JobType, const FString&, ObjectiveDescription);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSideJobCompleted, ESideJobType, JobType, int32, CashEarned, int32, TipEarned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSideJobFailed, ESideJobType, JobType, const FString&, Reason);

UCLASS()
class PROJECTREALWORLD_API URealWorldSideJobsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "SideJobs|Actions")
    bool StartJob(ESideJobType JobType, AActor* RequiredVehicle);

    UFUNCTION(BlueprintCallable, Category = "SideJobs|Actions")
    void CancelJob();

    UFUNCTION(BlueprintCallable, Category = "SideJobs|Actions")
    void PickUpPassenger();

    UFUNCTION(BlueprintCallable, Category = "SideJobs|Actions")
    void DeliverPassenger();

    UFUNCTION(BlueprintCallable, Category = "SideJobs|Actions")
    void ReportVehicleCollision(float ImpulseIntensity);

    // ─── Queries ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "SideJobs|State")
    bool IsJobActive() const { return ActiveSession.JobType != ESideJobType::None; }

    UFUNCTION(BlueprintPure, Category = "SideJobs|State")
    FSideJobSession GetCurrentSession() const { return ActiveSession; }

    // ─── SFX Assets ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SideJobs|Audio")
    TObjectPtr<USoundBase> JobStartSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SideJobs|Audio")
    TObjectPtr<USoundBase> CashRewardSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SideJobs|Audio")
    TObjectPtr<USoundBase> JobFailSound;

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "SideJobs|Events")
    FOnSideJobStarted OnSideJobStarted;

    UPROPERTY(BlueprintAssignable, Category = "SideJobs|Events")
    FOnSideJobCompleted OnSideJobCompleted;

    UPROPERTY(BlueprintAssignable, Category = "SideJobs|Events")
    FOnSideJobFailed OnSideJobFailed;

private:
    FTimerHandle JobTickerHandle;
    FSideJobSession ActiveSession;

    void TickJob();
    void GenerateNextObjective();
    void FailJob(const FString& Reason);
};
