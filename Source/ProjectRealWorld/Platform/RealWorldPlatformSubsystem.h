#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Platform/RealWorldScalabilityTypes.h"
#include "RealWorldPlatformSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGraphicsPresetChanged,         ERealWorldGraphicsPreset,    NewPreset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBatteryThermalModeChanged,     EBatteryThermalMode,         NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlatformPerformanceTierChanged, EPlatformPerformanceTier,   NewTier);

/**
 * URealWorldPlatformSubsystem
 * Android-ONLY platform abstraction.
 * Auto-detects device RAM/CPU on startup, selects Mobile_Low/Medium/High preset,
 * scales NPC/traffic budgets, manages battery saver / thermal throttle.
 */
UCLASS()
class PROJECTREALWORLD_API URealWorldPlatformSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Detected hardware info */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform|Hardware")
	FPlatformHardwareInfo HardwareInfo;

	/** Currently active graphics preset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform|Scalability")
	ERealWorldGraphicsPreset ActiveGraphicsPreset = ERealWorldGraphicsPreset::Auto;

	/** Battery / thermal mode */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform|Thermal")
	EBatteryThermalMode ActiveBatteryMode = EBatteryThermalMode::Balanced;

	// ─── Queries ─────────────────────────────────────────────────────────────
	/** Always returns true — this is Android-only */
	UFUNCTION(BlueprintPure, Category = "Platform")
	bool IsMobilePlatform() const { return true; }

	/** Always returns true — touch is the only input on Android */
	UFUNCTION(BlueprintPure, Category = "Platform")
	bool SupportsTouchControls() const { return true; }

	UFUNCTION(BlueprintPure, Category = "Platform")
	EPlatformPerformanceTier GetPerformanceTier() const { return HardwareInfo.PerformanceTier; }

	UFUNCTION(BlueprintPure, Category = "Platform")
	ERealWorldGraphicsPreset GetRecommendedGraphicsPreset() const { return HardwareInfo.RecommendedPreset; }

	// ─── Scalability ─────────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Platform|Scalability")
	void ApplyGraphicsPreset(ERealWorldGraphicsPreset NewPreset);

	UFUNCTION(BlueprintCallable, Category = "Platform|Thermal")
	void SetBatteryThermalMode(EBatteryThermalMode NewMode);

	// ─── Scaled World Budgets ─────────────────────────────────────────────────
	/** Max NPC crowd actors to spawn at once */
	UFUNCTION(BlueprintPure, Category = "Platform|Budgets")
	int32 GetScaledCrowdBudget() const;

	/** Max traffic vehicles to spawn at once */
	UFUNCTION(BlueprintPure, Category = "Platform|Budgets")
	int32 GetScaledTrafficDensity() const;

	/** Draw distance multiplier 0.35-0.90 based on tier */
	UFUNCTION(BlueprintPure, Category = "Platform|Budgets")
	float GetScaledDrawDistanceMultiplier() const;

	/** Target frame rate (30 or 60) */
	UFUNCTION(BlueprintPure, Category = "Platform|Budgets")
	int32 GetTargetFrameRate() const;

	// ─── Events ──────────────────────────────────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
	FOnGraphicsPresetChanged OnGraphicsPresetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
	FOnBatteryThermalModeChanged OnBatteryThermalModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
	FOnPlatformPerformanceTierChanged OnPlatformPerformanceTierChanged;

protected:
	void DetectAndroidHardware();
	EPlatformPerformanceTier DeterminePerformanceTier() const;
	ERealWorldGraphicsPreset DetermineRecommendedPreset(EPlatformPerformanceTier Tier) const;
	void ApplyScalabilityCvars(ERealWorldGraphicsPreset Preset, EBatteryThermalMode ThermalMode);

private:
	bool bHasInitializedHardware = false;
};
