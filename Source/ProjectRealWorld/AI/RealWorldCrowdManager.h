#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI/RealWorldCrowdTypes.h"
#include "RealWorldCrowdManager.generated.h"

class ARealWorldNPCCharacter;

UCLASS()
class PROJECTREALWORLD_API ARealWorldCrowdManager : public AActor
{
	GENERATED_BODY()

public:
	ARealWorldCrowdManager();

	virtual void Tick(float DeltaTime) override;

	/** Target crowd budget for the single city block vertical slice (20-30 NPCs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
	int32 TargetCrowdCount = 24;

	/** Array of active crowd member actors */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
	TArray<TObjectPtr<ARealWorldNPCCharacter>> ActiveCrowdMembers;

	/** Spawn points for crowd generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawning")
	TArray<FRealWorldCrowdSpawnPoint> DefinedSpawnPoints;

	/** Commercial store entrance and interior waypoints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Store")
	FVector StoreEntrancePoint = FVector(2000.0f, 950.0f, 16.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Store")
	FVector StoreShelfPoint = FVector(2200.0f, 1550.0f, 16.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Store")
	FVector StoreCounterPoint = FVector(1650.0f, 1450.0f, 16.0f);

	/** Populate the city block with the 20-30 diverse archetype crowd */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Crowd|Actions")
	void SpawnCityBlockCrowd();

	/** Broadcast danger alert to nearby crowd NPCs */
	UFUNCTION(BlueprintCallable, Category = "Crowd|Alerts")
	void AlertNearbyCrowd(const FVector& ThreatOrigin, float ThreatRadius);

	/** Send a civilian into the commercial convenience store */
	UFUNCTION(BlueprintCallable, Category = "Crowd|Store")
	void DispatchCivilianToStore();

protected:
	virtual void BeginPlay() override;

	void UpdateCrowdSimulationLODs();

private:
	float StoreDispatchTimer = 0.0f;
};
