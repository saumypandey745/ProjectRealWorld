#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Platform/RealWorldScalabilityTypes.h"
#include "RealWorldPlatformSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGraphicsPresetChanged, ERealWorldGraphicsPreset, NewPreset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBatteryThermalModeChanged, EBatteryThermalMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlatformPerformanceTierChanged, EPlatformPerformanceTier, NewTier);

/**
 * URealWorldPlatformSubsystem
 * Centralized, authoritative platform and hardware capability abstraction.
 * Auto-detects device specs on startup, determines appropriate performance tiers,
 * scales crowd/traffic budgets, and dispatches battery-saving thermal throttles.
 */
UCLASS()
class PROJECTREALWORLD_API URealWorldPlatformSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Hardware & Platform State */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform|Hardware")
	FPlatformHardwareInfo HardwareInfo;

	/** Currently Active Graphics Preset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform|Scalability")
	ERealWorldGraphicsPreset ActiveGraphicsPreset = ERealWorldGraphicsPreset::Auto;

	/** Active Battery & Thermal Throttling Mode */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform|Thermal")
	EBatteryThermalMode ActiveBatteryMode = EBatteryThermalMode::Balanced;

	// --- Platform Queries ---
	UFUNCTION(BlueprintPure, Category = "Platform")
	ERealWorldPlatformType GetPlatformType() const { return HardwareInfo.PlatformType; }

	UFUNCTION(BlueprintPure, Category = "Platform")
	EPlatformPerformanceTier GetPerformanceTier() const { return HardwareInfo.PerformanceTier; }

	UFUNCTION(BlueprintPure, Category = "Platform")
	bool IsMobilePlatform() const;

	UFUNCTION(BlueprintPure, Category = "Platform")
	bool IsPCOrEditor() const;

	UFUNCTION(BlueprintPure, Category = "Platform")
	bool SupportsTouchControls() const { return HardwareInfo.bSupportsTouch; }

	UFUNCTION(BlueprintPure, Category = "Platform")
	ERealWorldGraphicsPreset GetRecommendedGraphicsPreset() const { return HardwareInfo.RecommendedPreset; }

	// --- Scalability Controls ---
	UFUNCTION(BlueprintCallable, Category = "Platform|Scalability")
	void ApplyGraphicsPreset(ERealWorldGraphicsPreset NewPreset);

	UFUNCTION(BlueprintCallable, Category = "Platform|Thermal")
	void SetBatteryThermalMode(EBatteryThermalMode NewMode);

	// --- Dynamic Scaled Budgets for World Subsystems ---
	UFUNCTION(BlueprintPure, Category = "Platform|Budgets")
	int32 GetScaledCrowdBudget() const;

	UFUNCTION(BlueprintPure, Category = "Platform|Budgets")
	int32 GetScaledTrafficDensity() const;

	UFUNCTION(BlueprintPure, Category = "Platform|Budgets")
	float GetScaledDrawDistanceMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Platform|Budgets")
	int32 GetTargetFrameRate() const;

	// --- Events ---
	UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
	FOnGraphicsPresetChanged OnGraphicsPresetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
	FOnBatteryThermalModeChanged OnBatteryThermalModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Platform|Events")
	FOnPlatformPerformanceTierChanged OnPlatformPerformanceTierChanged;

protected:
	void DetectPlatformAndCapabilities();
	EPlatformPerformanceTier DeterminePerformanceTier() const;
	ERealWorldGraphicsPreset DetermineRecommendedPreset(EPlatformPerformanceTier Tier) const;
	void ApplyScalabilityCvars(ERealWorldGraphicsPreset Preset, EBatteryThermalMode ThermalMode);

private:
	bool bHasInitializedHardware = false;
};
