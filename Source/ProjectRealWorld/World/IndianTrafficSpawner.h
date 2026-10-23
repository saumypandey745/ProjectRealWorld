#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/IndianCityTypes.h"
#include "IndianTrafficSpawner.generated.h"

class ARealWorldVehicleBase;

// ─── Vehicle Spawn Entry ─────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FIndianVehicleSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ARealWorldVehicleBase> VehicleClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EIndianVehicleType VehicleType = EIndianVehicleType::Maruti_Swift;

	// Spawn weight — higher = spawns more often
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
	float SpawnWeight = 0.3f;

	// Only spawn in these districts
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EIndianDistrict> AllowedDistricts;

	// Not allowed at this time range (e.g., tractors only in day)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinHour = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHour = 24.0f;
};

UCLASS()
class PROJECTREALWORLD_API AIndianTrafficSpawner : public AActor
{
	GENERATED_BODY()

public:
	AIndianTrafficSpawner();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ─── Config ───────────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indian Traffic")
	TArray<FIndianVehicleSpawnEntry> VehiclePool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indian Traffic")
	EIndianDistrict CurrentDistrict = EIndianDistrict::Commercial;

	// Max vehicles spawned at one time (keep low for 4GB RAM)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indian Traffic")
	int32 MaxSpawnedVehicles = 15;

	// Spawn distance from player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indian Traffic")
	float SpawnRadius = 8000.0f;

	// Despawn distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indian Traffic")
	float DespawnRadius = 12000.0f;

	// Current traffic density 0-1 (driven by time of day)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indian Traffic")
	float CurrentTrafficDensity = 0.5f;

	UFUNCTION(BlueprintCallable, Category = "Indian Traffic")
	void SetDistrict(EIndianDistrict NewDistrict);

	UFUNCTION(BlueprintCallable, Category = "Indian Traffic")
	void UpdateTrafficForTimeOfDay(float CurrentHour);

	UFUNCTION(BlueprintPure, Category = "Indian Traffic")
	int32 GetActiveVehicleCount() const { return SpawnedVehicles.Num(); }

private:
	UPROPERTY()
	TArray<TObjectPtr<ARealWorldVehicleBase>> SpawnedVehicles;

	float SpawnCheckTimer = 0.0f;
	float DespawnCheckTimer = 0.0f;

	void TrySpawnVehicle();
	void DespawnFarVehicles();
	TSubclassOf<ARealWorldVehicleBase> SelectVehicleClass(float CurrentHour) const;
	float GetTrafficDensityForHour(float Hour) const;
};
