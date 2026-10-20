#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WeatherSubsystem.h"
#include "RealWorldAtmosphereController.generated.h"

class UDirectionalLightComponent;
class USkyLightComponent;
class UExponentialHeightFogComponent;
class UPostProcessComponent;

UCLASS()
class PROJECTREALWORLD_API ARealWorldAtmosphereController : public AActor
{
	GENERATED_BODY()

public:
	ARealWorldAtmosphereController();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere|Lighting")
	TObjectPtr<UDirectionalLightComponent> SunLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere|Lighting")
	TObjectPtr<UDirectionalLightComponent> MoonLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere|Lighting")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere|Fog")
	TObjectPtr<UExponentialHeightFogComponent> HeightFog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere|PostProcess")
	TObjectPtr<UPostProcessComponent> PostProcessComp;

	/** Peak midday sun lux intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Config")
	float MaxSunIntensityLux = 110000.0f;

	/** Cool moonlight intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Config")
	float MaxMoonIntensityLux = 0.5f;

protected:
	virtual void BeginPlay() override;

	void UpdateCelestialLighting(float DeltaTime);
	void UpdateWeatherAtmosphere(float DeltaTime);
	void ProcessLightningStorm(float DeltaTime);

private:
	float LightningTimer = 0.0f;
	float LightningFlashDuration = 0.0f;
	bool bIsFlashActive = false;
};
