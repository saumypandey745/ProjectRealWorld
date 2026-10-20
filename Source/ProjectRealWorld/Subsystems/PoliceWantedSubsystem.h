#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PoliceWantedSubsystem.generated.h"

class ARealWorldNPCCharacter;
class ARealWorldVehicleBase;

UENUM(BlueprintType)
enum class ECrimeType : uint8
{
	MinorTheft UMETA(DisplayName = "Minor Theft"),
	RecklessDriving UMETA(DisplayName = "Reckless Driving"),
	Assault UMETA(DisplayName = "Assault"),
	GrandTheftAuto UMETA(DisplayName = "Grand Theft Auto"),
	DischargingFirearm UMETA(DisplayName = "Discharging Firearm"),
	Homicide UMETA(DisplayName = "Homicide")
};

UENUM(BlueprintType)
enum class EPursuitState : uint8
{
	Calm UMETA(DisplayName = "Calm"),
	Investigating UMETA(DisplayName = "Investigating Scene"),
	ActivePursuit UMETA(DisplayName = "Active Pursuit"),
	SearchingLastKnownArea UMETA(DisplayName = "Searching Last Known Area"),
	ArrestingSuspect UMETA(DisplayName = "Arresting Suspect"),
	Cooldown UMETA(DisplayName = "Cooldown / Evasion")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWantedStateUpdated, int32, WantedLevel, EPursuitState, PursuitState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSearchAreaChanged, FVector, Center, float, Radius);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPoliceDispatched, FVector, DispatchTargetLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerArrested);

UCLASS()
class PROJECTREALWORLD_API UPoliceWantedSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Wanted Level: 0 = None, 1 = Suspicious, 2 = Pursuit, 3 = Heavy Patrols, 4 = Tactical/Roadblocks, 5 = Maximum Force */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Law|State")
	int32 WantedLevel = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Law|State")
	EPursuitState CurrentPursuitState = EPursuitState::Calm;

	/** Accumulated crime severity heat */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Law|State")
	float CrimeSeverityAccumulator = 0.0f;

	/** Coordinates of crime scene or last confirmed sighting */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Law|SearchArea")
	FVector LastKnownPlayerLocation = FVector::ZeroVector;

	/** Dynamic radius of search sphere (expands over time when lost) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Law|SearchArea")
	float CurrentSearchRadius = 3500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Law|SearchArea")
	float MaxSearchRadius = 12000.0f;

	/** Time remaining before police call off the search when suspect remains unseen */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Law|Evasion")
	float EvasionTimer = 0.0f;

	/** Whether any responding unit currently has direct line-of-sight to player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Law|Sight")
	bool bPlayerInSight = false;

	/** Dispatch arrival delay (simulates response travel time) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Law|Dispatch")
	float DispatchArrivalTimer = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Law|Dispatch")
	bool bDispatchActive = false;

	// --- Actions ---
	UFUNCTION(BlueprintCallable, Category = "Law|Dispatch")
	void ReportCrime(ECrimeType Crime, FVector Location, AActor* Perpetrator, bool bDirectPoliceSight);

	UFUNCTION(BlueprintCallable, Category = "Law|Sight")
	void UpdateSuspectVisual(FVector CurrentLocation);

	UFUNCTION(BlueprintCallable, Category = "Law|Sight")
	void LoseSuspectVisual();

	UFUNCTION(BlueprintCallable, Category = "Law|Control")
	void ClearWantedLevel();

	UFUNCTION(BlueprintCallable, Category = "Law|Arrest")
	bool CanArrestPlayer(FVector OfficerLocation, bool bPlayerArmed) const;

	UFUNCTION(BlueprintCallable, Category = "Law|Arrest")
	void TriggerPlayerArrest();

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Law|Events")
	FOnWantedStateUpdated OnWantedStateUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Law|Events")
	FOnSearchAreaChanged OnSearchAreaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Law|Events")
	FOnPoliceDispatched OnPoliceDispatched;

	UPROPERTY(BlueprintAssignable, Category = "Law|Events")
	FOnPlayerArrested OnPlayerArrested;

private:
	float BaseEvasionDuration = 30.0f;
	void RecalculateThreatLevel(ECrimeType NewCrime);
	void SpawnNaturalPolicePatrol(const FVector& CrimeLocation);
};
