#include "Subsystems/TimeOfDaySubsystem.h"
#include "Kismet/KismetMathLibrary.h"

void UTimeOfDaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LastRecordedHour = FMath::FloorToInt(CurrentTimeHours);
	bIsNightTime = (CurrentTimeHours >= 20.0f || CurrentTimeHours < 6.0f);
}

void UTimeOfDaySubsystem::Tick(float DeltaTime)
{
	if (DayLengthInSeconds <= 0.0f) return;

	// Advance time
	const float HoursPerSecond = 24.0f / DayLengthInSeconds;
	CurrentTimeHours += HoursPerSecond * DeltaTime;

	if (CurrentTimeHours >= 24.0f)
	{
		CurrentTimeHours = FMath::Fmod(CurrentTimeHours, 24.0f);
	}

	// Hour change notification
	const int32 CurrentHour = FMath::FloorToInt(CurrentTimeHours);
	if (CurrentHour != LastRecordedHour)
	{
		LastRecordedHour = CurrentHour;
		OnTimeHourPassed.Broadcast(CurrentHour);

		const bool bNewNight = (CurrentTimeHours >= 20.0f || CurrentTimeHours < 6.0f);
		if (bNewNight != bIsNightTime)
		{
			bIsNightTime = bNewNight;
			OnNightStateChanged.Broadcast(bIsNightTime);
		}
	}
}

TStatId UTimeOfDaySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTimeOfDaySubsystem, STATGROUP_Tickables);
}

FVector UTimeOfDaySubsystem::GetSunDirection() const
{
	// Sun angle: 0h = midnight (-90 pitch), 6h = sunrise (0 pitch), 12h = noon (90 pitch), 18h = sunset (180 pitch)
	const float TimeFraction = CurrentTimeHours / 24.0f;
	const float SunPitch = (TimeFraction * 360.0f) - 90.0f;
	const FRotator SunRotation(SunPitch, 65.0f, 0.0f);
	return SunRotation.Vector();
}

FVector UTimeOfDaySubsystem::GetMoonDirection() const
{
	// Moon is roughly opposite the sun
	const FVector SunDir = GetSunDirection();
	return -SunDir;
}

FString UTimeOfDaySubsystem::GetFormattedTimeString() const
{
	int32 Hours = FMath::FloorToInt(CurrentTimeHours);
	int32 Minutes = FMath::FloorToInt((CurrentTimeHours - Hours) * 60.0f);
	return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
}

void UTimeOfDaySubsystem::SetTimeOfDay(float NewHours)
{
	CurrentTimeHours = FMath::Clamp(NewHours, 0.0f, 23.99f);
	LastRecordedHour = FMath::FloorToInt(CurrentTimeHours);
	bIsNightTime = (CurrentTimeHours >= 20.0f || CurrentTimeHours < 6.0f);
	OnNightStateChanged.Broadcast(bIsNightTime);
}
