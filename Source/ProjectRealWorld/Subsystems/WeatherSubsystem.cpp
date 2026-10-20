#include "Subsystems/WeatherSubsystem.h"
#include "Kismet/KismetMathLibrary.h"

void UWeatherSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentWeather = EWeatherType::Clear;
	CurrentProperties = GetPresetForWeather(CurrentWeather);
	TargetProperties = CurrentProperties;
	TransitionAlpha = 1.0f;
	RoadWetness = 0.0f;
}

void UWeatherSubsystem::Tick(float DeltaTime)
{
	// Interpolate weather properties smoothly
	if (TransitionAlpha < 1.0f)
	{
		const float Step = DeltaTime / FMath::Max(0.1f, TransitionDuration);
		TransitionAlpha = FMath::Clamp(TransitionAlpha + Step, 0.0f, 1.0f);

		CurrentProperties.RainIntensity = FMath::Lerp(CurrentProperties.RainIntensity, TargetProperties.RainIntensity, TransitionAlpha);
		CurrentProperties.CloudCover = FMath::Lerp(CurrentProperties.CloudCover, TargetProperties.CloudCover, TransitionAlpha);
		CurrentProperties.FogDensity = FMath::Lerp(CurrentProperties.FogDensity, TargetProperties.FogDensity, TransitionAlpha);
		CurrentProperties.WindSpeed = FMath::Lerp(CurrentProperties.WindSpeed, TargetProperties.WindSpeed, TransitionAlpha);
		CurrentProperties.VisibilityRange = FMath::Lerp(CurrentProperties.VisibilityRange, TargetProperties.VisibilityRange, TransitionAlpha);
		CurrentProperties.RoadFrictionMultiplier = FMath::Lerp(CurrentProperties.RoadFrictionMultiplier, TargetProperties.RoadFrictionMultiplier, TransitionAlpha);
	}

	// Dynamic road drying and wetting physics
	if (CurrentProperties.RainIntensity > 0.05f)
	{
		// Accumulate puddle wetness
		RoadWetness = FMath::Clamp(RoadWetness + (CurrentProperties.RainIntensity * 0.05f * DeltaTime), 0.0f, 1.0f);
	}
	else
	{
		// Dry out gradually
		RoadWetness = FMath::Clamp(RoadWetness - (0.015f * DeltaTime), 0.0f, 1.0f);
	}

	OnRoadWetnessUpdated.Broadcast(RoadWetness);
}

TStatId UWeatherSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWeatherSubsystem, STATGROUP_Tickables);
}

void UWeatherSubsystem::SetTargetWeather(EWeatherType NewWeather)
{
	if (CurrentWeather == NewWeather) return;

	CurrentWeather = NewWeather;
	TargetProperties = GetPresetForWeather(NewWeather);
	TransitionAlpha = 0.0f;

	OnWeatherChanged.Broadcast(CurrentWeather);
}

FWeatherProperties UWeatherSubsystem::GetPresetForWeather(EWeatherType Weather) const
{
	FWeatherProperties P;
	switch (Weather)
	{
	case EWeatherType::Clear:
		P.RainIntensity = 0.0f;
		P.CloudCover = 0.15f;
		P.FogDensity = 0.00005f;
		P.WindSpeed = 8.0f;
		P.VisibilityRange = 60000.0f;
		P.RoadFrictionMultiplier = 1.0f;
		break;
	case EWeatherType::Cloudy:
		P.RainIntensity = 0.0f;
		P.CloudCover = 0.85f;
		P.FogDensity = 0.0002f;
		P.WindSpeed = 15.0f;
		P.VisibilityRange = 40000.0f;
		P.RoadFrictionMultiplier = 0.98f;
		break;
	case EWeatherType::Rain:
		P.RainIntensity = 0.45f;
		P.CloudCover = 0.95f;
		P.FogDensity = 0.0008f;
		P.WindSpeed = 25.0f;
		P.VisibilityRange = 25000.0f;
		P.RoadFrictionMultiplier = 0.78f;
		break;
	case EWeatherType::HeavyRain:
		P.RainIntensity = 0.85f;
		P.CloudCover = 1.0f;
		P.FogDensity = 0.0018f;
		P.WindSpeed = 45.0f;
		P.VisibilityRange = 12000.0f;
		P.RoadFrictionMultiplier = 0.65f;
		break;
	case EWeatherType::Storm:
		P.RainIntensity = 1.0f;
		P.CloudCover = 1.0f;
		P.FogDensity = 0.003f;
		P.WindSpeed = 70.0f;
		P.VisibilityRange = 8000.0f;
		P.RoadFrictionMultiplier = 0.55f;
		break;
	case EWeatherType::Fog:
		P.RainIntensity = 0.0f;
		P.CloudCover = 0.6f;
		P.FogDensity = 0.008f;
		P.WindSpeed = 3.0f;
		P.VisibilityRange = 4000.0f;
		P.RoadFrictionMultiplier = 0.92f;
		break;
	}
	return P;
}
