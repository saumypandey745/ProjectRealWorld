#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/IndianCityTypes.h"
#include "IndianEnvironmentController.generated.h"

class UDirectionalLightComponent;
class USkyLightComponent;
class UExponentialHeightFogComponent;
class UAudioComponent;

// ─── Indian Season Weather Properties ────────────────────────────────────────
USTRUCT(BlueprintType)
struct FIndianSeasonProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EIndianSeason Season = EIndianSeason::PostMonsoon;

	// Fog density (high in sardi/winter)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin="0.0", ClampMax="1.0"))
	float FogDensity = 0.02f;

	// Rain probability 0-1 (high in monsoon)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin="0.0", ClampMax="1.0"))
	float RainProbability = 0.0f;

	// Heat haze intensity (high in garmi/summer)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin="0.0", ClampMax="1.0"))
	float HeatHazeIntensity = 0.0f;

	// Sun temperature in Kelvin
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SunTemperatureK = 6500.0f;

	// Sky saturation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin="0.0", ClampMax="2.0"))
	float SkySaturation = 1.0f;

	// Ambient wind speed (affects foliage, cloth, dust)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WindSpeedKmH = 10.0f;
};

UCLASS()
class PROJECTREALWORLD_API AIndianEnvironmentController : public AActor
{
	GENERATED_BODY()

public:
	AIndianEnvironmentController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ─── Season Configuration ─────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indian Environment")
	EIndianSeason CurrentSeason = EIndianSeason::PostMonsoon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indian Environment")
	TArray<FIndianSeasonProperties> SeasonProfiles;

	// ─── Current Active Properties (interpolated) ─────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indian Environment|State")
	float CurrentRainIntensity = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indian Environment|State")
	float CurrentFogDensity = 0.02f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indian Environment|State")
	float CurrentHeatHaze = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indian Environment|State")
	float CurrentTemperatureC = 28.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indian Environment|State")
	float CurrentWindSpeed = 10.0f;

	// ─── References ───────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indian Environment|Refs")
	TObjectPtr<UDirectionalLightComponent> SunLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indian Environment|Refs")
	TObjectPtr<UExponentialHeightFogComponent> AtmosphericFog;

	// ─── Actions ─────────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Indian Environment")
	void SetSeason(EIndianSeason NewSeason);

	UFUNCTION(BlueprintCallable, Category = "Indian Environment")
	void TriggerMonsoonRain();

	UFUNCTION(BlueprintCallable, Category = "Indian Environment")
	void TriggerWinterFog();

	UFUNCTION(BlueprintCallable, Category = "Indian Environment")
	void StopRain();

	UFUNCTION(BlueprintPure, Category = "Indian Environment")
	float GetTemperatureDisplay() const { return CurrentTemperatureC; }

	UFUNCTION(BlueprintPure, Category = "Indian Environment")
	FText GetSeasonDisplayName() const;

	UFUNCTION(BlueprintCallable, Category = "Indian Environment")
	void UpdateTimeOfDayLighting(float CurrentHour);

private:
	float TargetRainIntensity = 0.0f;
	float TargetFogDensity = 0.02f;
	float WeatherTransitionSpeed = 0.3f;

	FIndianSeasonProperties GetSeasonProfile(EIndianSeason Season) const;
	void ApplySeasonToEnvironment(const FIndianSeasonProperties& Props);
	FLinearColor GetSunColorForTime(float Hour) const;
	float GetSunIntensityForTime(float Hour) const;
};
