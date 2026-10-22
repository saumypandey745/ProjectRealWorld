#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Platform/RealWorldScalabilityTypes.h"
#include "RealWorldSettingsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsUpdated, const FCustomGraphicsSettings&, NewSettings);

/**
 * URealWorldSettingsSubsystem
 * Provides comprehensive data-driven bindings for the PC and Mobile Graphics Settings menu.
 * Integrates with UGameUserSettings and handles disk persistence.
 */
UCLASS()
class PROJECTREALWORLD_API URealWorldSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Settings")
	FCustomGraphicsSettings ActiveSettings;

	UFUNCTION(BlueprintCallable, Category = "Settings|Preset")
	void SetGraphicsPreset(ERealWorldGraphicsPreset Preset);

	UFUNCTION(BlueprintCallable, Category = "Settings|Resolution")
	void SetResolutionMode(ERealWorldResolutionMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Settings|Resolution")
	ERealWorldResolutionMode GetResolutionMode() const { return ActiveSettings.ResolutionMode; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Quality")
	void SetResolutionScale(float NewScale);

	UFUNCTION(BlueprintCallable, Category = "Settings|Quality")
	void SetViewDistanceQuality(int32 Quality);

	UFUNCTION(BlueprintCallable, Category = "Settings|Quality")
	void SetShadowQuality(int32 Quality);

	UFUNCTION(BlueprintCallable, Category = "Settings|Quality")
	void SetPostProcessQuality(int32 Quality);

	UFUNCTION(BlueprintCallable, Category = "Settings|Quality")
	void SetTextureQuality(int32 Quality);

	UFUNCTION(BlueprintCallable, Category = "Settings|Quality")
	void SetEffectsQuality(int32 Quality);

	UFUNCTION(BlueprintCallable, Category = "Settings|Quality")
	void SetFoliageQuality(int32 Quality);

	UFUNCTION(BlueprintCallable, Category = "Settings|Quality")
	void SetGlobalIlluminationQuality(int32 Quality);

	UFUNCTION(BlueprintCallable, Category = "Settings|Quality")
	void SetReflectionQuality(int32 Quality);

	UFUNCTION(BlueprintCallable, Category = "Settings|Display")
	void SetTargetFPS(int32 NewFPS);

	UFUNCTION(BlueprintCallable, Category = "Settings|Simulation")
	void SetNPCDensityMultiplier(float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "Settings|Simulation")
	void SetTrafficDensityMultiplier(float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "Settings|Display")
	void SetVSyncEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Mobile")
	void SetBatteryThermalMode(EBatteryThermalMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Settings|Persistence")
	void ResetToPlatformDefaults();

	UFUNCTION(BlueprintCallable, Category = "Settings|Persistence")
	void SaveSettingsToDisk();

	UFUNCTION(BlueprintCallable, Category = "Settings|Persistence")
	void LoadSettingsFromDisk();

	UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
	FOnSettingsUpdated OnSettingsUpdated;

private:
	void ApplySettingsToEngine();
};
