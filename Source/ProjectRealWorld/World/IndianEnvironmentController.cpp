// ============================================================================
// IndianEnvironmentController.cpp
// Project RealWorld — Indian Seasonal Environment System
// Handles: Summer heat haze, Monsoon rains, Winter fog, Pleasant autumn
// Indian time-of-day: warm golden sunrise, harsh midday white, crimson sunset
// ============================================================================

#include "World/IndianEnvironmentController.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AIndianEnvironmentController::AIndianEnvironmentController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f; // 10Hz — no need for per-frame

	// Default season profiles
	SeasonProfiles.Add(FIndianSeasonProperties());
	SeasonProfiles[0].Season           = EIndianSeason::Summer;
	SeasonProfiles[0].FogDensity       = 0.005f;
	SeasonProfiles[0].RainProbability  = 0.0f;
	SeasonProfiles[0].HeatHazeIntensity = 0.7f;
	SeasonProfiles[0].SunTemperatureK  = 7500.0f; // Bright white-hot sun
	SeasonProfiles[0].SkySaturation    = 0.8f;
	SeasonProfiles[0].WindSpeedKmH     = 5.0f;

	FIndianSeasonProperties Monsoon;
	Monsoon.Season           = EIndianSeason::Monsoon;
	Monsoon.FogDensity       = 0.04f;
	Monsoon.RainProbability  = 0.85f;
	Monsoon.HeatHazeIntensity = 0.0f;
	Monsoon.SunTemperatureK  = 5500.0f; // Overcast, cooler
	Monsoon.SkySaturation    = 1.2f;    // Greens pop after rain
	Monsoon.WindSpeedKmH     = 25.0f;
	SeasonProfiles.Add(Monsoon);

	FIndianSeasonProperties PostMonsoon;
	PostMonsoon.Season           = EIndianSeason::PostMonsoon;
	PostMonsoon.FogDensity       = 0.01f;
	PostMonsoon.RainProbability  = 0.1f;
	PostMonsoon.HeatHazeIntensity = 0.1f;
	PostMonsoon.SunTemperatureK  = 6500.0f; // Golden pleasant
	PostMonsoon.SkySaturation    = 1.1f;
	PostMonsoon.WindSpeedKmH     = 12.0f;
	SeasonProfiles.Add(PostMonsoon);

	FIndianSeasonProperties Winter;
	Winter.Season           = EIndianSeason::Winter;
	Winter.FogDensity       = 0.08f;  // Dense north-Indian winter fog
	Winter.RainProbability  = 0.05f;
	Winter.HeatHazeIntensity = 0.0f;
	Winter.SunTemperatureK  = 5800.0f; // Cooler, muted
	Winter.SkySaturation    = 0.85f;
	Winter.WindSpeedKmH     = 8.0f;
	SeasonProfiles.Add(Winter);
}

void AIndianEnvironmentController::BeginPlay()
{
	Super::BeginPlay();
	SetSeason(CurrentSeason);
}

// ─────────────────────────────────────────────────────────────────────────────
// TICK — Smooth weather transitions
// ─────────────────────────────────────────────────────────────────────────────
void AIndianEnvironmentController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Smooth interpolation towards target rain/fog
	CurrentRainIntensity = FMath::FInterpTo(
		CurrentRainIntensity, TargetRainIntensity, DeltaTime, WeatherTransitionSpeed
	);

	CurrentFogDensity = FMath::FInterpTo(
		CurrentFogDensity, TargetFogDensity, DeltaTime, WeatherTransitionSpeed * 0.5f
	);

	// Apply to fog component
	if (AtmosphericFog)
	{
		AtmosphericFog->SetFogDensity(CurrentFogDensity);
		AtmosphericFog->SetFogInscatteringColor(
			FLinearColor::LerpUsingHSV(
				FLinearColor(0.7f, 0.75f, 0.8f), // Clear sky blue-grey
				FLinearColor(0.4f, 0.45f, 0.5f), // Monsoon dark grey
				CurrentRainIntensity
			)
		);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// SET SEASON
// ─────────────────────────────────────────────────────────────────────────────
void AIndianEnvironmentController::SetSeason(EIndianSeason NewSeason)
{
	CurrentSeason = NewSeason;
	const FIndianSeasonProperties Props = GetSeasonProfile(NewSeason);
	ApplySeasonToEnvironment(Props);

	switch (NewSeason)
	{
	case EIndianSeason::Summer:
		CurrentTemperatureC = 40.0f;
		break;
	case EIndianSeason::Monsoon:
		CurrentTemperatureC = 28.0f;
		TriggerMonsoonRain();
		break;
	case EIndianSeason::PostMonsoon:
		CurrentTemperatureC = 25.0f;
		break;
	case EIndianSeason::Winter:
		CurrentTemperatureC = 10.0f;
		TriggerWinterFog();
		break;
	}
}

void AIndianEnvironmentController::ApplySeasonToEnvironment(const FIndianSeasonProperties& Props)
{
	TargetFogDensity = Props.FogDensity;
	CurrentWindSpeed = Props.WindSpeedKmH;
	CurrentHeatHaze  = Props.HeatHazeIntensity;

	if (AtmosphericFog)
	{
		AtmosphericFog->SetFogDensity(Props.FogDensity);
	}

	if (SunLight)
	{
		SunLight->SetLightColor(FLinearColor::MakeFromColorTemperature(Props.SunTemperatureK));
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// MONSOON / FOG TRIGGERS
// ─────────────────────────────────────────────────────────────────────────────
void AIndianEnvironmentController::TriggerMonsoonRain()
{
	TargetRainIntensity = 0.9f;
	TargetFogDensity    = 0.04f;
}

void AIndianEnvironmentController::TriggerWinterFog()
{
	TargetFogDensity    = 0.1f; // Dense north Indian "kaafi fog"
	TargetRainIntensity = 0.0f;
}

void AIndianEnvironmentController::StopRain()
{
	TargetRainIntensity = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// TIME OF DAY LIGHTING — Indian Sun Colors
// ─────────────────────────────────────────────────────────────────────────────
void AIndianEnvironmentController::UpdateTimeOfDayLighting(float CurrentHour)
{
	if (!SunLight) return;

	const FLinearColor SunColor    = GetSunColorForTime(CurrentHour);
	const float        SunIntensity = GetSunIntensityForTime(CurrentHour);

	SunLight->SetLightColor(SunColor);
	SunLight->SetIntensity(SunIntensity);

	// Sun angle: -90 at midnight, 0 at 6am (sunrise), 90 at noon, 180 at 6pm (sunset)
	const float SunPitchDeg = ((CurrentHour - 6.0f) / 12.0f) * 180.0f - 90.0f;
	SunLight->SetRelativeRotation(FRotator(SunPitchDeg, 45.0f, 0.0f));
}

FLinearColor AIndianEnvironmentController::GetSunColorForTime(float Hour) const
{
	// Indian sun color palette
	if (Hour < 5.5f || Hour > 20.0f) // Night — moonlight blue
		return FLinearColor(0.05f, 0.07f, 0.15f);
	if (Hour < 7.0f) // Sunrise — warm orange-pink (India sunrises are vivid)
		return FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.35f, 0.1f), FLinearColor(1.0f, 0.75f, 0.4f), (Hour - 5.5f) / 1.5f);
	if (Hour < 10.0f) // Morning — warm gold
		return FLinearColor(1.0f, 0.9f, 0.7f);
	if (Hour < 14.0f) // Harsh midday — India has very white, hot sun
		return FLinearColor(1.0f, 0.97f, 0.9f);
	if (Hour < 17.0f) // Afternoon — slightly warm
		return FLinearColor(1.0f, 0.92f, 0.8f);
	if (Hour < 19.0f) // Sunset — spectacular Indian crimson-orange
		return FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.8f, 0.4f), FLinearColor(0.9f, 0.2f, 0.05f), (Hour - 17.0f) / 2.0f);
	// Dusk — dark blue
	return FLinearColor(0.15f, 0.1f, 0.2f);
}

float AIndianEnvironmentController::GetSunIntensityForTime(float Hour) const
{
	if (Hour < 6.0f || Hour > 20.0f) return 0.2f;    // Moonlight
	if (Hour < 7.0f) return FMath::Lerp(0.2f, 3.0f, (Hour - 6.0f));    // Sunrise
	if (Hour < 10.0f) return FMath::Lerp(3.0f, 8.0f, (Hour - 7.0f) / 3.0f); // Morning
	if (Hour < 15.0f) return 10.0f;  // Harsh Indian midday peak
	if (Hour < 18.0f) return FMath::Lerp(10.0f, 4.0f, (Hour - 15.0f) / 3.0f); // Afternoon
	return FMath::Lerp(4.0f, 0.2f, (Hour - 18.0f) / 2.0f); // Sunset to dusk
}

FIndianSeasonProperties AIndianEnvironmentController::GetSeasonProfile(EIndianSeason Season) const
{
	for (const FIndianSeasonProperties& P : SeasonProfiles)
	{
		if (P.Season == Season) return P;
	}
	return FIndianSeasonProperties();
}

FText AIndianEnvironmentController::GetSeasonDisplayName() const
{
	switch (CurrentSeason)
	{
	case EIndianSeason::Summer:     return FText::FromString(TEXT("Garmi (Summer)"));
	case EIndianSeason::Monsoon:    return FText::FromString(TEXT("Barish (Monsoon)"));
	case EIndianSeason::PostMonsoon: return FText::FromString(TEXT("Sharad (Post-Monsoon)"));
	case EIndianSeason::Winter:     return FText::FromString(TEXT("Sardi (Winter)"));
	default:                        return FText::FromString(TEXT("Unknown"));
	}
}
