#include "World/IndianMapGenerator.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "World/IndianTrafficSpawner.h"
#include "World/IndianEnvironmentController.h"

AIndianMapGenerator::AIndianMapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	MapRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MapRoot"));
	SetRootComponent(MapRoot);

	// Setup Instanced Mesh Components for 4GB Android Mobile Optimization
	RoadMeshInstancer = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RoadMeshInstancer"));
	RoadMeshInstancer->SetupAttachment(MapRoot);
	RoadMeshInstancer->SetCollisionProfileName(TEXT("BlockAll"));
	RoadMeshInstancer->SetCastShadow(false); // Mobile optimization

	DividerMeshInstancer = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("DividerMeshInstancer"));
	DividerMeshInstancer->SetupAttachment(MapRoot);
	DividerMeshInstancer->SetCollisionProfileName(TEXT("BlockAll"));
	DividerMeshInstancer->SetCastShadow(true);

	SidewalkMeshInstancer = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SidewalkMeshInstancer"));
	SidewalkMeshInstancer->SetupAttachment(MapRoot);
	SidewalkMeshInstancer->SetCollisionProfileName(TEXT("BlockAll"));

	BuildingMeshInstancer = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BuildingMeshInstancer"));
	BuildingMeshInstancer->SetupAttachment(MapRoot);
	BuildingMeshInstancer->SetCollisionProfileName(TEXT("BlockAll"));
	BuildingMeshInstancer->SetCastShadow(true);

	RoofWaterTankInstancer = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RoofWaterTankInstancer"));
	RoofWaterTankInstancer->SetupAttachment(MapRoot);

	StreetLightInstancer = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("StreetLightInstancer"));
	StreetLightInstancer->SetupAttachment(MapRoot);

	ChaiTapriInstancer = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ChaiTapriInstancer"));
	ChaiTapriInstancer->SetupAttachment(MapRoot);
	ChaiTapriInstancer->SetCollisionProfileName(TEXT("BlockAll"));

	ElectricPoleInstancer = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ElectricPoleInstancer"));
	ElectricPoleInstancer->SetupAttachment(MapRoot);

	TreeFoliageInstancer = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TreeFoliageInstancer"));
	TreeFoliageInstancer->SetupAttachment(MapRoot);
}

void AIndianMapGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AIndianMapGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoBuildOnBeginPlay)
	{
		GenerateCompleteIndianMap();
		ConnectToTrafficAndEnvironmentSubsystems();
	}
}

void AIndianMapGenerator::ClearGeneratedMap()
{
	if (RoadMeshInstancer) RoadMeshInstancer->ClearInstances();
	if (DividerMeshInstancer) DividerMeshInstancer->ClearInstances();
	if (SidewalkMeshInstancer) SidewalkMeshInstancer->ClearInstances();
	if (BuildingMeshInstancer) BuildingMeshInstancer->ClearInstances();
	if (RoofWaterTankInstancer) RoofWaterTankInstancer->ClearInstances();
	if (StreetLightInstancer) StreetLightInstancer->ClearInstances();
	if (ChaiTapriInstancer) ChaiTapriInstancer->ClearInstances();
	if (ElectricPoleInstancer) ElectricPoleInstancer->ClearInstances();
	if (TreeFoliageInstancer) TreeFoliageInstancer->ClearInstances();

	TotalBuildingsSpawned = 0;
	TotalRoadSegmentsSpawned = 0;
	TotalPropsSpawned = 0;
}

void AIndianMapGenerator::GenerateCompleteIndianMap()
{
	ClearGeneratedMap();

	UE_LOG(LogTemp, Warning, TEXT("[IndianMapGenerator] Starting Week 1 Indian Open World Map Generation..."));

	BuildTerrainAndDistricts();
	BuildRoadNetwork();
	BuildHighwayCorridor();
	BuildMarineDriveAndCoast();
	BuildOldCityBazaar();
	BuildSlumAndChawlCluster();
	BuildFarmlandsAndVillage();
	BuildGhatHillsAndElevation();
	SpawnIndianStreetProps();

	UE_LOG(LogTemp, Warning, TEXT("[IndianMapGenerator] Map Generation Complete! Spawned %d Buildings, %d Roads, %d Props."),
		TotalBuildingsSpawned, TotalRoadSegmentsSpawned, TotalPropsSpawned);
}

void AIndianMapGenerator::BuildTerrainAndDistricts()
{
	// Setup base ground foundation for the 600m x 600m open world map
	const FVector GroundScale = FVector(WorldSizeX / 100.0f, WorldSizeY / 100.0f, 1.0f);
	const FTransform GroundTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, -50.0f), GroundScale);

	if (RoadMeshInstancer)
	{
		RoadMeshInstancer->AddInstance(GroundTransform);
	}
}

void AIndianMapGenerator::BuildRoadNetwork()
{
	if (!RoadMeshInstancer || !SidewalkMeshInstancer) return;

	// 1. Central City Main Avenue (North-South): 400 meters long, 16m wide
	const float CityRoadLength = 40000.0f;
	const int32 NumSegments = 40;
	const float SegmentLen = CityRoadLength / NumSegments;

	for (int32 i = 0; i < NumSegments; ++i)
	{
		const float YPos = -20000.0f + (i * SegmentLen);
		const FTransform RoadSegTransform(FRotator::ZeroRotator, FVector(0.0f, YPos, 0.0f), FVector(SegmentLen / 100.0f, 16.0f, 0.2f));
		RoadMeshInstancer->AddInstance(RoadSegTransform);

		// Concrete Divider with yellow/black curb paint
		if (DividerMeshInstancer && (i % 2 == 0))
		{
			const FTransform DivTransform(FRotator::ZeroRotator, FVector(0.0f, YPos, 20.0f), FVector(SegmentLen / 100.0f, 0.8f, 0.4f));
			DividerMeshInstancer->AddInstance(DivTransform);
		}

		// Sidewalks on Left and Right
		const FTransform LeftSidewalk(FRotator::ZeroRotator, FVector(-900.0f, YPos, 15.0f), FVector(SegmentLen / 100.0f, 2.5f, 0.3f));
		const FTransform RightSidewalk(FRotator::ZeroRotator, FVector(900.0f, YPos, 15.0f), FVector(SegmentLen / 100.0f, 2.5f, 0.3f));
		SidewalkMeshInstancer->AddInstance(LeftSidewalk);
		SidewalkMeshInstancer->AddInstance(RightSidewalk);

		TotalRoadSegmentsSpawned++;
	}

	// 2. East-West Cross Avenues (Connecting Bazaar to Slum to Beach)
	for (int32 Cross = -2; Cross <= 2; ++Cross)
	{
		const float CrossY = Cross * 8000.0f;
		for (int32 j = -15; j <= 15; ++j)
		{
			const float XPos = j * 1000.0f;
			const FTransform CrossRoadSeg(FRotator(0.0f, 90.0f, 0.0f), FVector(XPos, CrossY, 0.0f), FVector(10.0f, 12.0f, 0.2f));
			RoadMeshInstancer->AddInstance(CrossRoadSeg);
			TotalRoadSegmentsSpawned++;
		}
	}
}

void AIndianMapGenerator::BuildHighwayCorridor()
{
	if (!bSpawnHighwayCorridor || !RoadMeshInstancer || !DividerMeshInstancer) return;

	// National Highway (NH-48 style): 4-lane wide elevated corridor along North side
	const float HighwayY = 16000.0f;
	for (int32 k = -20; k <= 20; ++k)
	{
		const float X = k * 1200.0f;
		// Elevated flyover sections
		const float Z = (FMath::Abs(k) < 10) ? 600.0f : 0.0f;

		// 4-lane highway asphalt
		const FTransform HighwaySeg(FRotator(0.0f, 90.0f, 0.0f), FVector(X, HighwayY, Z), FVector(12.0f, 24.0f, 0.3f));
		RoadMeshInstancer->AddInstance(HighwaySeg);

		// Center Concrete Jersey Barrier Divider
		const FTransform JerseyBarrier(FRotator(0.0f, 90.0f, 0.0f), FVector(X, HighwayY, Z + 30.0f), FVector(12.0f, 0.6f, 0.8f));
		DividerMeshInstancer->AddInstance(JerseyBarrier);

		// Streetlights every 4 segments
		if (StreetLightInstancer && (k % 4 == 0))
		{
			const FTransform LightLeft(FRotator(0.0f, 0.0f, 0.0f), FVector(X, HighwayY - 1250.0f, Z + 10.0f), FVector(1.0f, 1.0f, 1.5f));
			const FTransform LightRight(FRotator(0.0f, 180.0f, 0.0f), FVector(X, HighwayY + 1250.0f, Z + 10.0f), FVector(1.0f, 1.0f, 1.5f));
			StreetLightInstancer->AddInstance(LightLeft);
			StreetLightInstancer->AddInstance(LightRight);
			TotalPropsSpawned += 2;
		}

		TotalRoadSegmentsSpawned++;
	}
}

void AIndianMapGenerator::BuildMarineDriveAndCoast()
{
	if (!bSpawnBeachZone || !RoadMeshInstancer) return;

	// West Coast Coastal Road & Marine Drive Promenade (X: -22000.0f)
	const float CoastX = -22000.0f;
	for (int32 i = -18; i <= 18; ++i)
	{
		const float Y = i * 1200.0f;

		// Curved shoreline road
		const float OffsetX = CoastX + (FMath::Sin(i * 0.2f) * 800.0f);
		const FTransform PromRoad(FRotator(0.0f, 0.0f, 0.0f), FVector(OffsetX, Y, 50.0f), FVector(12.0f, 18.0f, 0.25f));
		RoadMeshInstancer->AddInstance(PromRoad);

		// Sea-wall promenade sidewalk with sea view
		if (SidewalkMeshInstancer)
		{
			const FTransform SeaWalk(FRotator::ZeroRotator, FVector(OffsetX - 1100.0f, Y, 80.0f), FVector(12.0f, 5.0f, 0.6f));
			SidewalkMeshInstancer->AddInstance(SeaWalk);
		}

		// Palm trees along beach promenade
		if (TreeFoliageInstancer && (i % 3 == 0))
		{
			const FTransform PalmTree(FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f), FVector(OffsetX - 1500.0f, Y, 20.0f), FVector(1.2f, 1.2f, 1.5f));
			TreeFoliageInstancer->AddInstance(PalmTree);
			TotalPropsSpawned++;
		}
	}
}

void AIndianMapGenerator::BuildOldCityBazaar()
{
	if (!bSpawnOldCityBazaar || !BuildingMeshInstancer) return;

	// Dense Old City Bazaar: Narrow lanes lined with 2-4 floor Indian shop-houses
	// Ground floor = Kirana, Chai, Medical, Sweet shop; 1st-3rd floor = Apartments
	for (int32 Row = -6; Row <= 6; ++Row)
	{
		for (int32 Col = 1; Col <= 4; ++Col)
		{
			const float BuildingY = Row * 2200.0f;
			const float LeftX = -1400.0f - (Col * 1800.0f);
			const float RightX = 1400.0f + (Col * 1800.0f);

			// Left Side Bazaar Shophouse
			const int32 FloorsLeft = FMath::RandRange(2, 4);
			const FVector BuildingScaleLeft(15.0f, 18.0f, FloorsLeft * 3.5f);
			const FTransform BldgLeft(FRotator::ZeroRotator, FVector(LeftX, BuildingY, 0.0f), BuildingScaleLeft);
			BuildingMeshInstancer->AddInstance(BldgLeft);
			TotalBuildingsSpawned++;

			// Black Sintex Water Tank on roof
			if (RoofWaterTankInstancer)
			{
				const FTransform TankTransform(FRotator::ZeroRotator, FVector(LeftX + 200.0f, BuildingY + 200.0f, FloorsLeft * 350.0f), FVector(1.0f, 1.0f, 1.2f));
				RoofWaterTankInstancer->AddInstance(TankTransform);
				TotalPropsSpawned++;
			}

			// Right Side Bazaar Shophouse
			const int32 FloorsRight = FMath::RandRange(2, 4);
			const FVector BuildingScaleRight(15.0f, 18.0f, FloorsRight * 3.5f);
			const FTransform BldgRight(FRotator::ZeroRotator, FVector(RightX, BuildingY, 0.0f), BuildingScaleRight);
			BuildingMeshInstancer->AddInstance(BldgRight);
			TotalBuildingsSpawned++;

			if (RoofWaterTankInstancer)
			{
				const FTransform TankTransform(FRotator::ZeroRotator, FVector(RightX - 200.0f, BuildingY - 200.0f, FloorsRight * 350.0f), FVector(1.0f, 1.0f, 1.2f));
				RoofWaterTankInstancer->AddInstance(TankTransform);
				TotalPropsSpawned++;
			}
		}
	}
}

void AIndianMapGenerator::BuildSlumAndChawlCluster()
{
	if (!bSpawnSlumCluster || !BuildingMeshInstancer) return;

	// East Sector: Slum & Chawl Cluster (Compact 1-2 floor houses with tin roofs)
	const float SlumBaseX = 12000.0f;
	for (int32 r = -8; r <= 8; ++r)
	{
		for (int32 c = 0; c < 5; ++c)
		{
			const float X = SlumBaseX + (c * 1100.0f) + FMath::RandRange(-80.0f, 80.0f);
			const float Y = (r * 1200.0f) + FMath::RandRange(-60.0f, 60.0f);
			const int32 Floors = FMath::RandRange(1, 2);

			const FVector SlumScale(8.0f, 9.0f, Floors * 2.8f);
			const FTransform SlumTransform(FRotator(0.0f, FMath::RandRange(-15.0f, 15.0f), 0.0f), FVector(X, Y, 0.0f), SlumScale);
			BuildingMeshInstancer->AddInstance(SlumTransform);
			TotalBuildingsSpawned++;
		}
	}
}

void AIndianMapGenerator::BuildFarmlandsAndVillage()
{
	if (!bSpawnFarmlands || !BuildingMeshInstancer) return;

	// South Sector: Farmlands with Khet, Village huts, and Tractor Sheds (Y: -22000.0f)
	const float FarmBaseY = -22000.0f;
	for (int32 fx = -8; fx <= 8; ++fx)
	{
		const float X = fx * 2800.0f;
		const float Y = FarmBaseY + FMath::RandRange(-1500.0f, 1500.0f);

		// Village Mud House / Farm House
		const FVector HutScale(12.0f, 14.0f, 3.0f);
		const FTransform HutTransform(FRotator(0.0f, FMath::RandRange(0.0f, 180.0f), 0.0f), FVector(X, Y, 0.0f), HutScale);
		BuildingMeshInstancer->AddInstance(HutTransform);
		TotalBuildingsSpawned++;

		// Green Neem & Banyan trees around farmland
		if (TreeFoliageInstancer)
		{
			const FTransform FarmTree(FRotator::ZeroRotator, FVector(X + 600.0f, Y + 600.0f, 0.0f), FVector(1.5f, 1.5f, 1.8f));
			TreeFoliageInstancer->AddInstance(FarmTree);
			TotalPropsSpawned++;
		}
	}
}

void AIndianMapGenerator::BuildGhatHillsAndElevation()
{
	if (!bSpawnGhatHills || !BuildingMeshInstancer) return;

	// North-East Sector: Ghat Hills Elevation with Viewpoint Fort
	const float HillBaseY = 24000.0f;
	for (int32 h = -6; h <= 6; ++h)
	{
		const float X = h * 3000.0f;
		const float Z = 400.0f + (FMath::Abs(h) * 200.0f); // Ascending elevation

		// Hilltop Fort / Mandir / Lookout Tower
		const FVector HillStructureScale(18.0f, 18.0f, 6.0f);
		const FTransform HillBldg(FRotator::ZeroRotator, FVector(X, HillBaseY, Z), HillStructureScale);
		BuildingMeshInstancer->AddInstance(HillBldg);
		TotalBuildingsSpawned++;
	}
}

void AIndianMapGenerator::SpawnIndianStreetProps()
{
	// Roadside Chai Tapris & Street Vendor Spots
	if (ChaiTapriInstancer)
	{
		const TArray<FVector> ChaiLocations = {
			FVector(-1050.0f, -4000.0f, 15.0f),
			FVector(1050.0f, 2000.0f, 15.0f),
			FVector(-1050.0f, 6000.0f, 15.0f),
			FVector(1050.0f, -8000.0f, 15.0f),
			FVector(-21000.0f, 0.0f, 60.0f),    // Beach Chaiwala
			FVector(11500.0f, -2000.0f, 0.0f)   // Slum Chaiwala
		};

		for (const FVector& Loc : ChaiLocations)
		{
			const FTransform TapriTransform(FRotator(0.0f, 90.0f, 0.0f), Loc, FVector(3.0f, 2.5f, 2.2f));
			ChaiTapriInstancer->AddInstance(TapriTransform);
			TotalPropsSpawned++;
		}
	}

	// Overhead Electric Poles with Wire Bundles
	if (ElectricPoleInstancer)
	{
		for (int32 p = -8; p <= 8; ++p)
		{
			const float Y = p * 2500.0f;
			const FTransform PoleLeft(FRotator::ZeroRotator, FVector(-950.0f, Y, 15.0f), FVector(0.5f, 0.5f, 4.0f));
			const FTransform PoleRight(FRotator::ZeroRotator, FVector(950.0f, Y, 15.0f), FVector(0.5f, 0.5f, 4.0f));
			ElectricPoleInstancer->AddInstance(PoleLeft);
			ElectricPoleInstancer->AddInstance(PoleRight);
			TotalPropsSpawned += 2;
		}
	}
}

void AIndianMapGenerator::ConnectToTrafficAndEnvironmentSubsystems()
{
	// Auto-find IndianTrafficSpawner in the world and supply spawn routes
	TArray<AActor*> FoundSpawners;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AIndianTrafficSpawner::StaticClass(), FoundSpawners);

	if (FoundSpawners.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[IndianMapGenerator] Connected Indian Map Roads to %d IndianTrafficSpawner(s)."), FoundSpawners.Num());
	}

	// Auto-find IndianEnvironmentController
	TArray<AActor*> FoundEnvControllers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AIndianEnvironmentController::StaticClass(), FoundEnvControllers);

	if (FoundEnvControllers.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[IndianMapGenerator] Connected Indian Map to IndianEnvironmentController."));
	}
}

EIndianDistrict AIndianMapGenerator::GetDistrictAtLocation(FVector WorldLocation) const
{
	// West Coast: Beach / Marine Drive
	if (WorldLocation.X < -15000.0f)
	{
		return EIndianDistrict::Beach;
	}
	// North Ghat Hills
	if (WorldLocation.Y > 18000.0f)
	{
		return EIndianDistrict::TouristSpot;
	}
	// South Farmlands & Village
	if (WorldLocation.Y < -15000.0f)
	{
		return (WorldLocation.X > 5000.0f) ? EIndianDistrict::Farm : EIndianDistrict::Village;
	}
	// East Sector: Slums / Chawls
	if (WorldLocation.X > 8000.0f)
	{
		return EIndianDistrict::Slum;
	}
	// Center North: National Highway
	if (FMath::Abs(WorldLocation.Y - 16000.0f) < 2500.0f)
	{
		return EIndianDistrict::Highway;
	}
	// Central City: Old Bazaar & Low-Mid Residential
	if (FMath::Abs(WorldLocation.X) < 4000.0f)
	{
		return EIndianDistrict::OldCity;
	}

	return EIndianDistrict::ResidentialMid;
}
