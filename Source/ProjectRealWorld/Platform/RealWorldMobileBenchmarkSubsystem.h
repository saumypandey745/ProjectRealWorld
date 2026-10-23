#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RealWorldMobileBenchmarkSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBenchmarkCompleted, bool, bPassed4GBTarget);

/**
 * Benchmark Results Struct for 4GB Android Target
 */
USTRUCT(BlueprintType)
struct FMobileBenchmarkSummary
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float AverageFPS = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float OnePercentLowFPS = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 PeakMemoryMB = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 AverageDrawCalls = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bTarget30FpsAchieved = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bMemoryWithin4GBBudget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FString BenchmarkGrade = TEXT("PASS (Smooth 30 FPS on 4GB RAM)");
};

/**
 * URealWorldMobileBenchmarkSubsystem
 * Week 4 Core Deliverable:
 * Android Gameplay Testing & Automated Benchmarking System.
 * Validates smooth 30 FPS execution and memory compliance on 4GB / 64GB Android devices.
 */
UCLASS()
class PROJECTREALWORLD_API URealWorldMobileBenchmarkSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// FTickableGameObject overrides
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return bIsBenchmarkRunning; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(URealWorldMobileBenchmarkSubsystem, STATGROUP_Tickables); }

	// ─── Benchmark Control ───────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Benchmark")
	void StartAutomatedMobileBenchmark(float DurationSeconds = 30.0f);

	UFUNCTION(BlueprintCallable, Category = "Benchmark")
	void StopBenchmark();

	UFUNCTION(BlueprintPure, Category = "Benchmark")
	bool IsBenchmarkActive() const { return bIsBenchmarkRunning; }

	UFUNCTION(BlueprintPure, Category = "Benchmark")
	FMobileBenchmarkSummary GetLatestBenchmarkSummary() const { return LatestSummary; }

	UFUNCTION(BlueprintPure, Category = "Benchmark|Telemetry")
	float GetCurrentFPS() const { return CurrentFPS; }

	UFUNCTION(BlueprintPure, Category = "Benchmark|Telemetry")
	int32 GetCurrentMemoryUsageMB() const;

	UPROPERTY(BlueprintAssignable, Category = "Benchmark|Events")
	FOnBenchmarkCompleted OnBenchmarkCompleted;

private:
	bool bIsBenchmarkRunning = false;
	float BenchmarkTimer = 0.0f;
	float TargetDuration = 30.0f;

	float CurrentFPS = 60.0f;
	TArray<float> SampledFPS;
	int32 PeakMemorySampled = 0;

	FMobileBenchmarkSummary LatestSummary;

	void FinalizeBenchmarkResults();
};
