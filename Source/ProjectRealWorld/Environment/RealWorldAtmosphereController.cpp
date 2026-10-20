#include "Environment/RealWorldAtmosphereController.h"
#include "Subsystems/TimeOfDaySubsystem.h"
#include "Subsystems/WeatherSubsystem.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

ARealWorldAtmosphereController::ARealWorldAtmosphereController()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("AtmosphereRoot"));
	SetRootComponent(RootComp);

	// Sun Light
	SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	SunLight->SetupAttachment(RootComp);
	SunLight->SetIntensity(110000.0f); // Lux
	SunLight->bUsedAsAtmosphereSunLight = true;
	SunLight->AtmosphereSunLightIndex = 0;
	SunLight->bEnableLightShaftBloom = true;
	SunLight->CastShadows = true;

	// Moon Light
	MoonLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));
	MoonLight->SetupAttachment(RootComp);
	MoonLight->SetIntensity(0.5f);
	MoonLight->bUsedAsAtmosphereSunLight = true;
	MoonLight->AtmosphereSunLightIndex = 1;
	MoonLight->SetLightColor(FLinearColor(0.65f, 0.75f, 0.95f));
	MoonLight->CastShadows = true;

	// Sky Light (Realtime ambient bounce)
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(RootComp);
	SkyLight->bRealTimeCapture = true;
	SkyLight->SetIntensity(1.0f);

	// Exponential Height Fog
	HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
	HeightFog->SetupAttachment(RootComp);
	HeightFog->SetFogDensity(0.0001f);
	HeightFog->bEnableVolumetricFog = true;

	// Unbound Post Process Volume (Lumen Exposure & Color Grading)
	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComp"));
	PostProcessComp->SetupAttachment(RootComp);
	PostProcessComp->bUnbound = true;
	PostProcessComp->Settings.bOverride_AutoExposureMethod = true;
	PostProcessComp->Settings.AutoExposureMethod = AEM_Histogram;
	PostProcessComp->Settings.bOverride_AutoExposureMinBrightness = true;
	PostProcessComp->Settings.AutoExposureMinBrightness = 0.03f;
	PostProcessComp->Settings.bOverride_AutoExposureMaxBrightness = true;
	PostProcessComp->Settings.AutoExposureMaxBrightness = 15.0f;

	MaxSunIntensityLux = 110000.0f;
	MaxMoonIntensityLux = 0.5f;
	LightningTimer = 10.0f;
	bIsFlashActive = false;
}

void ARealWorldAtmosphereController::BeginPlay()
{
	Super::BeginPlay();
}

void ARealWorldAtmosphereController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCelestialLighting(DeltaTime);
	UpdateWeatherAtmosphere(DeltaTime);
	ProcessLightningStorm(DeltaTime);
}

void ARealWorldAtmosphereController::UpdateCelestialLighting(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UTimeOfDaySubsystem* TimeSub = World->GetSubsystem<UTimeOfDaySubsystem>();
	if (!TimeSub) return;

	const float CurrentHour = TimeSub->CurrentTimeHours;

	// Orient sun & moon
	const FVector SunDir = TimeSub->GetSunDirection();
	SunLight->SetWorldRotation(SunDir.Rotation());

	const FVector MoonDir = TimeSub->GetMoonDirection();
	MoonLight->SetWorldRotation(MoonDir.Rotation());

	// Calculate sun altitude factor [-1.0 at midnight, 1.0 at noon]
	const float AltitudeFactor = FMath::Clamp(SunDir.Z, -1.0f, 1.0f);

	if (AltitudeFactor > 0.0f)
	{
		// Daytime
		const float SunPower = FMath::Clamp(AltitudeFactor / 0.7f, 0.0f, 1.0f);
		SunLight->SetIntensity(FMath::Lerp(0.0f, MaxSunIntensityLux, SunPower));

		// Color temperature transition: warm golden sunrise/sunset (3200K) to midday white (6500K)
		if (AltitudeFactor < 0.25f)
		{
			// Golden hour amber/rose
			SunLight->SetLightColor(FLinearColor(1.0f, 0.65f, 0.45f));
		}
		else
		{
			SunLight->SetLightColor(FLinearColor(1.0f, 0.98f, 0.92f));
		}

		MoonLight->SetIntensity(0.0f);
	}
	else
	{
		// Nighttime
		SunLight->SetIntensity(0.0f);
		const float MoonPower = FMath::Clamp(-AltitudeFactor / 0.5f, 0.0f, 1.0f);
		MoonLight->SetIntensity(FMath::Lerp(0.0f, MaxMoonIntensityLux, MoonPower));
	}
}

void ARealWorldAtmosphereController::UpdateWeatherAtmosphere(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UWeatherSubsystem* WeatherSub = World->GetSubsystem<UWeatherSubsystem>();
	if (!WeatherSub) return;

	const FWeatherProperties& P = WeatherSub->CurrentProperties;

	// Cloud cover dimming
	const float CloudDimming = FMath::Lerp(1.0f, 0.25f, P.CloudCover);
	SunLight->SetIntensity(SunLight->Intensity * CloudDimming);

	// Fog interpolation
	if (HeightFog)
	{
		HeightFog->SetFogDensity(FMath::FInterpTo(HeightFog->FogDensity, P.FogDensity, DeltaTime, 1.5f));
	}
}

void ARealWorldAtmosphereController::ProcessLightningStorm(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UWeatherSubsystem* WeatherSub = World->GetSubsystem<UWeatherSubsystem>();
	if (!WeatherSub || WeatherSub->CurrentWeather != EWeatherType::Storm)
	{
		bIsFlashActive = false;
		return;
	}

	if (bIsFlashActive)
	{
		LightningFlashDuration -= DeltaTime;
		if (LightningFlashDuration <= 0.0f)
		{
			bIsFlashActive = false;
		}
	}
	else
	{
		LightningTimer -= DeltaTime;
		if (LightningTimer <= 0.0f)
		{
			// Trigger sudden high-intensity lightning burst
			bIsFlashActive = true;
			LightningFlashDuration = 0.08f; // 80ms flash
			LightningTimer = FMath::FRandRange(6.0f, 18.0f); // Next flash in 6-18s

			if (SunLight)
			{
				SunLight->SetIntensity(MaxSunIntensityLux * 2.5f);
				SunLight->SetLightColor(FLinearColor(0.85f, 0.92f, 1.0f));
			}
		}
	}
}
