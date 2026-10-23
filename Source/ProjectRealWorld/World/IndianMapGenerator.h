#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/IndianCityTypes.h"
#include "IndianMapGenerator.generated.h"

class UInstancedStaticMeshComponent;
class USplineComponent;
class UBoxComponent;
class AIndianTrafficSpawner;
class AIndianEnvironmentController;

/**
 * Procedural Indian Road Segment Definition
 */
USTRUCT(BlueprintType)
struct FIndianRoadSegment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	EIndianStreetTag RoadType = EIndianStreetTag::MainRoad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	FVector StartPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	FVector EndPoint = FVector(5000.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	float RoadWidth = 1400.0f; // 14 meters for 2-lane city avenue

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	bool bHasDivider = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	bool bHasSidewalk = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	int32 PotholeCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	int32 StreetlightCount = 6;
};

/**
 * Procedural Indian Building Lot Definition
 */
USTRUCT(BlueprintType)
struct FIndianBuildingLot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	EIndianDistrict District = EIndianDistrict::ResidentialLow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	FTransform Transform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 Floors = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	FVector Dimensions = FVector(1200.0f, 1000.0f, 350.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	bool bHasGroundFloorShop = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	bool bHasWaterTankOnRoof = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	bool bHasOverheadWires = true;
};

/**
 * AIndianMapGenerator
 * Week 1 Core Architecture:
 * Generates and manages the entire open-world map layout:
 * 1. Indian Terrain & Zone Layout (Marine Drive Coast, Ghat Hills, Old Bazaar, Slums, Highway, Farmland).
 * 2. Multi-tier Indian Road Network (Highway, Main City Avenues, Narrow Gali Lanes, Kacha Tracks).
 * 3. Authentic Indian Modular Buildings & Architecture (Shop-houses, Chawls, High-rise, Huts).
 * 4. Roadside Props (Chai tapris, electric poles with wire bundles, speed breakers, street dogs, auto stands).
 *
 * Highly optimized for 4GB RAM Android devices using Hierarchical Instanced Static Meshes (HISM)
 * to keep draw calls below 250 and frame rates solid at 30-60 FPS.
 */
UCLASS()
class PROJECTREALWORLD_API AIndianMapGenerator : public AActor
{
	GENERATED_BODY()

public:
	AIndianMapGenerator();

	virtual void OnConstruction(const FTransform& Transform) override;

	// ─── Map Dimensions & Configuration ───────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Dimensions")
	float WorldSizeX = 60000.0f; // 600 meters x 600 meters core playable sector

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Dimensions")
	float WorldSizeY = 60000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	bool bAutoBuildOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	bool bSpawnBeachZone = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	bool bSpawnGhatHills = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	bool bSpawnSlumCluster = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	bool bSpawnOldCityBazaar = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	bool bSpawnHighwayCorridor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	bool bSpawnFarmlands = true;

	// ─── Instanced Mesh Components (Android Draw-Call Optimization) ───────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map|Components")
	TObjectPtr<USceneComponent> MapRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map|Instanced")
	TObjectPtr<UInstancedStaticMeshComponent> RoadMeshInstancer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map|Instanced")
	TObjectPtr<UInstancedStaticMeshComponent> DividerMeshInstancer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map|Instanced")
	TObjectPtr<UInstancedStaticMeshComponent> SidewalkMeshInstancer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map|Instanced")
	TObjectPtr<UInstancedStaticMeshComponent> BuildingMeshInstancer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map|Instanced")
	TObjectPtr<UInstancedStaticMeshComponent> RoofWaterTankInstancer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map|Instanced")
	TObjectPtr<UInstancedStaticMeshComponent> StreetLightInstancer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map|Instanced")
	TObjectPtr<UInstancedStaticMeshComponent> ChaiTapriInstancer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map|Instanced")
	TObjectPtr<UInstancedStaticMeshComponent> ElectricPoleInstancer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map|Instanced")
	TObjectPtr<UInstancedStaticMeshComponent> TreeFoliageInstancer;

	// ─── Generation Triggers ──────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Map|Actions")
	void GenerateCompleteIndianMap();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Map|Actions")
	void ClearGeneratedMap();

	/** Returns the district enum at any given world location */
	UFUNCTION(BlueprintPure, Category = "Map|Queries")
	EIndianDistrict GetDistrictAtLocation(FVector WorldLocation) const;

protected:
	virtual void BeginPlay() override;

private:
	// Sub-generators
	void BuildTerrainAndDistricts();
	void BuildRoadNetwork();
	void BuildHighwayCorridor();
	void BuildMarineDriveAndCoast();
	void BuildOldCityBazaar();
	void BuildSlumAndChawlCluster();
	void BuildFarmlandsAndVillage();
	void BuildGhatHillsAndElevation();
	void SpawnIndianStreetProps();
	void ConnectToTrafficAndEnvironmentSubsystems();

	int32 TotalBuildingsSpawned = 0;
	int32 TotalRoadSegmentsSpawned = 0;
	int32 TotalPropsSpawned = 0;
};
