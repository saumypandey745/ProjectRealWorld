#include "Subsystems/RealWorldAudioSubsystem.h"
#include "Subsystems/WeatherSubsystem.h"
#include "Engine/World.h"

void URealWorldAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentAcousticZone = EAudioAcousticZone::ExteriorOpenAir;
	OcclusionLPFFrequency = 20000.0f;
	TargetLPF = 20000.0f;
	MetaSoundRainIntensity = 0.0f;
}

void URealWorldAudioSubsystem::Tick(float DeltaTime)
{
	// Interpolate low-pass filter frequency for smooth audio muffled transitions
	if (!FMath::IsNearlyEqual(OcclusionLPFFrequency, TargetLPF, 10.0f))
	{
		OcclusionLPFFrequency = FMath::FInterpTo(OcclusionLPFFrequency, TargetLPF, DeltaTime, 8.0f);
	}

	// Sync rain intensity from Weather Subsystem for MetaSounds parameters
	if (UWorld* World = GetWorld())
	{
		if (UWeatherSubsystem* WeatherSub = World->GetSubsystem<UWeatherSubsystem>())
		{
			MetaSoundRainIntensity = WeatherSub->CurrentProperties.RainIntensity;
		}
	}
}

TStatId URealWorldAudioSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URealWorldAudioSubsystem, STATGROUP_Tickables);
}

void URealWorldAudioSubsystem::SetAcousticZone(EAudioAcousticZone NewZone)
{
	CurrentAcousticZone = NewZone;

	switch (NewZone)
	{
	case EAudioAcousticZone::ExteriorOpenAir:
		TargetLPF = 20000.0f;
		break;
	case EAudioAcousticZone::ExteriorUrbanAlley:
		TargetLPF = 16000.0f;
		break;
	case EAudioAcousticZone::InteriorSmallRoom:
		TargetLPF = 3500.0f; // Muffles outdoor traffic and sirens
		break;
	case EAudioAcousticZone::InteriorLargeHall:
		TargetLPF = 5000.0f;
		break;
	case EAudioAcousticZone::UndergroundTunnel:
		TargetLPF = 2200.0f;
		break;
	}
}

void URealWorldAudioSubsystem::SetListenerInteriorOcclusion(bool bIsIndoors, float OcclusionFactor)
{
	if (bIsIndoors)
	{
		TargetLPF = FMath::Lerp(20000.0f, 1500.0f, FMath::Clamp(OcclusionFactor, 0.0f, 1.0f));
	}
	else
	{
		TargetLPF = 20000.0f;
	}
}
