#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimeOfDaySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeHourPassed, int32, NewHour);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNightStateChanged, bool, bIsNight);

UCLASS()
class PROJECTREALWORLD_API UTimeOfDaySubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Real-world seconds per game day (e.g. 1440.0f = 24 minutes real time for a 24-hour day) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeOfDay|Config")
	float DayLengthInSeconds = 1440.0f;

	/** Current time of day in decimal hours [0.0 - 24.0) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeOfDay|State")
	float CurrentTimeHours = 8.5f;

	/** Is it currently night (between 20:00 and 06:00)? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeOfDay|State")
	bool bIsNightTime = false;

	/** Get sun direction vector for dynamic directional light alignment */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay|SunMoon")
	FVector GetSunDirection() const;

	/** Get moon direction vector */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay|SunMoon")
	FVector GetMoonDirection() const;

	/** Format current time as readable clock string (e.g., "14:35") */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Formatting")
	FString GetFormattedTimeString() const;

	/** Manually set time of day */
	UFUNCTION(BlueprintCallable, Category = "TimeOfDay|Control")
	void SetTimeOfDay(float NewHours);

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "TimeOfDay|Events")
	FOnTimeHourPassed OnTimeHourPassed;

	UPROPERTY(BlueprintAssignable, Category = "TimeOfDay|Events")
	FOnNightStateChanged OnNightStateChanged;

private:
	int32 LastRecordedHour = -1;
};
