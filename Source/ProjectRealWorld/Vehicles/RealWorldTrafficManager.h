#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealWorldTrafficManager.generated.h"

class USplineComponent;
class ARealWorldVehicleBase;
class ARealWorldCharacter;

UENUM(BlueprintType)
enum class ETrafficLightState : uint8
{
	Red UMETA(DisplayName = "Red"),
	Yellow UMETA(DisplayName = "Yellow"),
	Green UMETA(DisplayName = "Green")
};

UENUM(BlueprintType)
enum class ETrafficDensity : uint8
{
	Off UMETA(DisplayName = "Off"),
	Low UMETA(DisplayName = "Low"),
	Medium UMETA(DisplayName = "Medium"),
	High UMETA(DisplayName = "High"),
	RushHour UMETA(DisplayName = "Rush Hour")
};

UENUM(BlueprintType)
enum class ETrafficVehicleLOD : uint8
{
	Near UMETA(DisplayName = "Near - Full Physics"),
	Medium UMETA(DisplayName = "Medium - Reduced Tick"),
	Far UMETA(DisplayName = "Far - Kinematic Spline"),
	VeryFar UMETA(DisplayName = "Very Far - Abstract")
};

USTRUCT(BlueprintType)
struct FTrafficStopLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic|Intersection")
	FVector StopLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic|Intersection")
	FVector ForwardDirection = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic|Intersection")
	bool bGovernedByNorthSouthSignal = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic|Intersection")
	float StopToleranceDistance = 450.0f;
};

USTRUCT(BlueprintType)
struct FTrafficAgent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic|Agent")
	TWeakObjectPtr<ARealWorldVehicleBase> Vehicle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic|Agent")
	float DistanceAlongSpline = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic|Agent")
	float TargetSpeedKmH = 45.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic|Agent")
	float CurrentSpeedKmH = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic|Agent")
	ETrafficVehicleLOD LOD = ETrafficVehicleLOD::Near;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic|Agent")
	bool bIsEmergencyVehicle = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic|Agent")
	bool bYieldingToEmergency = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic|Agent")
	bool bWaitingAtRedLight = false;

	float TickAccumulator = 0.0f;
};

UCLASS()
class PROJECTREALWORLD_API ARealWorldTrafficManager : public AActor
{
	GENERATED_BODY()

public:
	ARealWorldTrafficManager();

	virtual void Tick(float DeltaTime) override;

	// --- Splines & Lanes ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic|Splines")
	TObjectPtr<USplineComponent> RoadSpline;

	// --- Traffic Signals ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic|Signals")
	ETrafficLightState NorthSouthSignal = ETrafficLightState::Green;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic|Signals")
	ETrafficLightState EastWestSignal = ETrafficLightState::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic|Signals")
	float GreenPhaseDuration = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic|Signals")
	float YellowPhaseDuration = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic|Intersection")
	TArray<FTrafficStopLine> StopLines;

	// --- Traffic Density & Spawning ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic|Density")
	ETrafficDensity DensitySetting = ETrafficDensity::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic|Spawning")
	TSubclassOf<ARealWorldVehicleBase> CivilianVehicleClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic|Spawning")
	float MinSpawnDistance = 3500.0f; // 35m from player

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic|Spawning")
	float MaxSpawnDistance = 9000.0f; // 90m from player

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic|Spawning")
	float DespawnDistance = 14000.0f; // 140m from player

	// --- Active Traffic Agents ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic|Simulation")
	TArray<FTrafficAgent> ActiveAgents;

	// --- Safety & Perception ---
	UFUNCTION(BlueprintPure, Category = "Traffic|Safety")
	bool CheckForwardObstacle(FVector Origin, FVector ForwardVector, float LookaheadDistance) const;

	UFUNCTION(BlueprintPure, Category = "Traffic|Safety")
	bool CheckPedestrianCrossingAhead(FVector Origin, FVector ForwardVector, float LookaheadDistance) const;

	UFUNCTION(BlueprintPure, Category = "Traffic|Safety")
	float ComputeSafeFollowingDistance(float CurrentSpeedKmH) const;

	UFUNCTION(BlueprintCallable, Category = "Traffic|Emergency")
	void NotifyEmergencyVehicleInArea(FVector EmergencyLocation, float SirenAudibleRadius);

	int32 GetMaxActiveVehiclesForDensity() const;

protected:
	virtual void BeginPlay() override;

	void UpdateTrafficSignals(float DeltaTime);
	void ManageVehicleSpawningAndDespawning(float DeltaTime);
	void SimulateTrafficAgents(float DeltaTime);
	void UpdateAgentLOD(FTrafficAgent& Agent, const FVector& PlayerLocation);

private:
	float SignalPhaseTimer = 0.0f;
	bool bNorthSouthIsActiveGreen = true;
	float SpawnCheckTimer = 0.0f;
};
