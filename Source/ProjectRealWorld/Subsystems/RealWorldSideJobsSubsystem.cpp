// ============================================================================
// RealWorldSideJobsSubsystem.cpp
// Project RealWorld — GTA-Class Side Hustles & Service Jobs
// ============================================================================

#include "Subsystems/RealWorldSideJobsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void URealWorldSideJobsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ActiveSession = FSideJobSession();
}

void URealWorldSideJobsSubsystem::Deinitialize()
{
    CancelJob();
    Super::Deinitialize();
}

bool URealWorldSideJobsSubsystem::StartJob(ESideJobType JobType, AActor* RequiredVehicle)
{
    if (JobType == ESideJobType::None) return false;

    ActiveSession = FSideJobSession();
    ActiveSession.JobType = JobType;
    ActiveSession.ConsecutiveCompleted = 0;
    ActiveSession.TotalCashEarned = 0;

    GenerateNextObjective();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            JobTickerHandle,
            this, &URealWorldSideJobsSubsystem::TickJob,
            1.0f,
            true
        );
    }

    if (JobStartSound)
    {
        UGameplayStatics::PlaySound2D(this, JobStartSound);
    }

    OnSideJobStarted.Broadcast(JobType, TEXT("Reach the highlighted customer pickup location!"));
    return true;
}

void URealWorldSideJobsSubsystem::CancelJob()
{
    if (ActiveSession.JobType == ESideJobType::None) return;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(JobTickerHandle);
    }

    ActiveSession.JobType = ESideJobType::None;
}

void URealWorldSideJobsSubsystem::GenerateNextObjective()
{
    ActiveSession.bHasPassenger = false;
    ActiveSession.CustomerSatisfaction = 1.0f;

    // Procedural landmark locations across Los Santos
    static const FVector CityPoints[] = {
        FVector(5000.0f, -8000.0f, 100.0f),    // Legion Square
        FVector(-15000.0f, -12000.0f, 120.0f), // Del Perro Pier
        FVector(-22000.0f, 5000.0f, 150.0f),   // Rockford Hills Luxury
        FVector(12000.0f, 18000.0f, 80.0f),    // Vinewood Boulevard
        FVector(25000.0f, -15000.0f, 60.0f),   // Terminal Harbor
        FVector(-8000.0f, -25000.0f, 90.0f)    // Los Santos International Airport
    };

    const int32 PkIdx = FMath::RandRange(0, 5);
    int32 DstIdx = FMath::RandRange(0, 5);
    if (DstIdx == PkIdx) DstIdx = (PkIdx + 1) % 6;

    ActiveSession.PickupLocation = CityPoints[PkIdx];
    ActiveSession.DestinationLocation = CityPoints[DstIdx];

    // Distance-based time limit (approx 45-90 seconds)
    const float Distance = FVector::Dist(ActiveSession.PickupLocation, ActiveSession.DestinationLocation);
    ActiveSession.RemainingTime = FMath::Clamp(Distance / 500.0f + 35.0f, 40.0f, 120.0f);
}

void URealWorldSideJobsSubsystem::TickJob()
{
    if (ActiveSession.JobType == ESideJobType::None) return;

    ActiveSession.RemainingTime -= 1.0f;

    if (ActiveSession.RemainingTime <= 0.0f)
    {
        FailJob(TEXT("Time expired! The customer left."));
    }
}

void URealWorldSideJobsSubsystem::PickUpPassenger()
{
    if (ActiveSession.JobType == ESideJobType::None || ActiveSession.bHasPassenger) return;

    ActiveSession.bHasPassenger = true;

    // Reset timer for destination delivery
    const float Dist = FVector::Dist(ActiveSession.PickupLocation, ActiveSession.DestinationLocation);
    ActiveSession.RemainingTime = FMath::Clamp(Dist / 500.0f + 30.0f, 40.0f, 120.0f);
}

void URealWorldSideJobsSubsystem::DeliverPassenger()
{
    if (ActiveSession.JobType == ESideJobType::None || !ActiveSession.bHasPassenger) return;

    // Calculate fare & tip
    const float BaseDistance = FVector::Dist(ActiveSession.PickupLocation, ActiveSession.DestinationLocation);
    const int32 BaseFare = FMath::RoundToInt(BaseDistance * 0.035f) + 150;
    const int32 Tip = FMath::RoundToInt(BaseFare * 0.4f * ActiveSession.CustomerSatisfaction);

    ActiveSession.ConsecutiveCompleted++;
    ActiveSession.TotalCashEarned += (BaseFare + Tip);

    if (CashRewardSound)
    {
        UGameplayStatics::PlaySound2D(this, CashRewardSound);
    }

    OnSideJobCompleted.Broadcast(ActiveSession.JobType, BaseFare, Tip);

    // Prepare next customer
    GenerateNextObjective();
}

void URealWorldSideJobsSubsystem::ReportVehicleCollision(float ImpulseIntensity)
{
    if (!ActiveSession.bHasPassenger) return;

    // Crashing annoys passenger, lowering tip
    const float Penalty = FMath::Clamp(ImpulseIntensity / 50000.0f, 0.05f, 0.35f);
    ActiveSession.CustomerSatisfaction = FMath::Max(ActiveSession.CustomerSatisfaction - Penalty, 0.0f);
}

void URealWorldSideJobsSubsystem::FailJob(const FString& Reason)
{
    if (JobFailSound)
    {
        UGameplayStatics::PlaySound2D(this, JobFailSound);
    }

    OnSideJobFailed.Broadcast(ActiveSession.JobType, Reason);
    CancelJob();
}
