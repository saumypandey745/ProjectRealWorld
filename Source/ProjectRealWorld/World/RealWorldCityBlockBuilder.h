#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealWorldCityBlockBuilder.generated.h"

class UStaticMeshComponent;
class ARealWorldModularBuilding;
class ARealWorldStreetProp;
class ARealWorldPCGEnvironment;
class ARealWorldAtmosphereController;
class ARealWorldNPCCharacter;
class ARealWorldPlayerStart;
class ARealWorldPickup;
class ARealWorldDoor;

/**
 * Procedural Master Constructor for the L_WorldPartition_City High-Fidelity Vertical Slice.
 * Assembles:
 * - Realistic asphalt roads with retro-reflective lane markings and crosswalk zebra bars.
 * - Gutter storm drains and concrete beveled curbs.
 * - Modular storefronts and detailed enterable commercial interiors with acoustic isolation.
 * - Scaled street furniture (lights, meters, fire hydrants, benches, dumpsters, planters).
 * - Deterministic PCG environmental dressing (sidewalk grass seams & gutter debris).
 * - Physical celestial atmosphere lighting controller (Sun/Moon/Fog/Storm).
 * - Ambient NPC citizen presence.
 */
UCLASS()
class PROJECTREALWORLD_API ARealWorldCityBlockBuilder : public AActor
{
	GENERATED_BODY()

public:
	ARealWorldCityBlockBuilder();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment|Root")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Config")
	float MainStreetLength = 12000.0f; // 120 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Config")
	float StreetWidth = 1400.0f; // 14 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Config")
	float SidewalkWidth = 300.0f; // 3 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Config")
	float CurbHeight = 16.0f; // 16 cm

	/** Build the complete realistic city block vertical slice */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment|Actions")
	void BuildVerticalSliceEnvironment();

protected:
	virtual void BeginPlay() override;

private:
	void SpawnRoadAndMarkings();
	void SpawnSidewalksAndCurbs();
	void SpawnBuildingsAndInteriors();
	void SpawnVaultTestObstacles();
	void SpawnStreetFurniture();
	void SpawnPCGAndAtmosphere();
	void SpawnActorsAndNPCs();
};
