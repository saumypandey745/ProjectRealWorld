#include "World/RealWorldCityBlockBuilder.h"
#include "World/RealWorldModularBuilding.h"
#include "World/RealWorldStreetProps.h"
#include "World/RealWorldPCGEnvironment.h"
#include "Environment/RealWorldAtmosphereController.h"
#include "AI/RealWorldNPCCharacter.h"
#include "AI/RealWorldCrowdManager.h"
#include "Core/RealWorldPlayerStart.h"
#include "Interaction/RealWorldDoor.h"
#include "Interaction/RealWorldPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

ARealWorldCityBlockBuilder::ARealWorldCityBlockBuilder()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MainStreetLength = 12000.0f; // 120 meters
	StreetWidth = 1400.0f;       // 14 meters
	SidewalkWidth = 300.0f;      // 3 meters
	CurbHeight = 16.0f;          // 16 cm
}

void ARealWorldCityBlockBuilder::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ARealWorldCityBlockBuilder::BeginPlay()
{
	Super::BeginPlay();
	BuildVerticalSliceEnvironment();
}

void ARealWorldCityBlockBuilder::BuildVerticalSliceEnvironment()
{
	SpawnRoadAndMarkings();
	SpawnSidewalksAndCurbs();
	SpawnBuildingsAndInteriors();
	SpawnVaultTestObstacles();
	SpawnStreetFurniture();
	SpawnPCGAndAtmosphere();
	SpawnActorsAndNPCs();
}

void ARealWorldCityBlockBuilder::SpawnRoadAndMarkings()
{
	auto CreateMesh = [this](const FVector& Pos, const FVector& Scale, const TCHAR* Name, const TCHAR* Profile = TEXT("BlockAll")) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this, Name);
		if (Comp)
		{
			Comp->RegisterComponent();
			Comp->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);
			Comp->SetRelativeLocation(Pos);
			Comp->SetRelativeScale3D(Scale);
			Comp->SetCollisionProfileName(Profile);
		}
		return Comp;
	};

	// 1. Main Street Asphalt (120m long x 14m wide)
	CreateMesh(FVector(0.0f, 0.0f, 0.0f), FVector(MainStreetLength / 100.0f, StreetWidth / 100.0f, 0.1f), TEXT("MainStreetAsphalt"));

	// 2. Cross Street Asphalt forming 4-way intersection (80m long x 14m wide)
	CreateMesh(FVector(0.0f, 0.0f, 0.0f), FVector(StreetWidth / 100.0f, 8000.0f / 100.0f, 0.1f), TEXT("CrossStreetAsphalt"));

	// 3. Center Double-Yellow Line Markings (East & West of intersection)
	CreateMesh(FVector(-3500.0f, 0.0f, 0.5f), FVector(50.0f, 0.15f, 0.02f), TEXT("YellowCenterLine_West"), TEXT("NoCollision"));
	CreateMesh(FVector(3500.0f, 0.0f, 0.5f), FVector(50.0f, 0.15f, 0.02f), TEXT("YellowCenterLine_East"), TEXT("NoCollision"));

	// 4. White Outer Lane Edge Markings
	const float EdgeY = (StreetWidth * 0.5f) - 60.0f;
	CreateMesh(FVector(-3500.0f, EdgeY, 0.5f), FVector(50.0f, 0.15f, 0.02f), TEXT("WhiteEdgeLine_NorthWest"), TEXT("NoCollision"));
	CreateMesh(FVector(3500.0f, EdgeY, 0.5f), FVector(50.0f, 0.15f, 0.02f), TEXT("WhiteEdgeLine_NorthEast"), TEXT("NoCollision"));
	CreateMesh(FVector(-3500.0f, -EdgeY, 0.5f), FVector(50.0f, 0.15f, 0.02f), TEXT("WhiteEdgeLine_SouthWest"), TEXT("NoCollision"));
	CreateMesh(FVector(3500.0f, -EdgeY, 0.5f), FVector(50.0f, 0.15f, 0.02f), TEXT("WhiteEdgeLine_SouthEast"), TEXT("NoCollision"));

	// 5. Pedestrian Zebra Crosswalk Bars at Intersection
	for (int32 i = -5; i <= 5; ++i)
	{
		const float BarY = i * 110.0f;
		CreateMesh(FVector(-800.0f, BarY, 0.6f), FVector(4.0f, 0.5f, 0.02f), *FString::Printf(TEXT("ZebraBar_West_%d"), i + 5), TEXT("NoCollision"));
		CreateMesh(FVector(800.0f, BarY, 0.6f), FVector(4.0f, 0.5f, 0.02f), *FString::Printf(TEXT("ZebraBar_East_%d"), i + 5), TEXT("NoCollision"));
	}

	// 6. Gutter Drainage Sewer Grates
	CreateMesh(FVector(-1500.0f, EdgeY + 20.0f, 0.4f), FVector(1.0f, 0.5f, 0.05f), TEXT("StormDrain1"), TEXT("BlockAll"));
	CreateMesh(FVector(1500.0f, EdgeY + 20.0f, 0.4f), FVector(1.0f, 0.5f, 0.05f), TEXT("StormDrain2"), TEXT("BlockAll"));
	CreateMesh(FVector(-1500.0f, -(EdgeY + 20.0f), 0.4f), FVector(1.0f, 0.5f, 0.05f), TEXT("StormDrain3"), TEXT("BlockAll"));
	CreateMesh(FVector(1500.0f, -(EdgeY + 20.0f), 0.4f), FVector(1.0f, 0.5f, 0.05f), TEXT("StormDrain4"), TEXT("BlockAll"));
}

void ARealWorldCityBlockBuilder::SpawnSidewalksAndCurbs()
{
	auto CreateMesh = [this](const FVector& Pos, const FVector& Scale, const TCHAR* Name)
	{
		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this, Name);
		if (Comp)
		{
			Comp->RegisterComponent();
			Comp->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);
			Comp->SetRelativeLocation(Pos);
			Comp->SetRelativeScale3D(Scale);
			Comp->SetCollisionProfileName(TEXT("BlockAll"));
		}
	};

	const float HalfStreet = StreetWidth * 0.5f;

	// North Sidewalk (West and East sections separated by intersection)
	CreateMesh(FVector(-3500.0f, HalfStreet + (SidewalkWidth * 0.5f), CurbHeight * 0.5f),
		FVector(50.0f, SidewalkWidth / 100.0f, CurbHeight / 100.0f), TEXT("Sidewalk_NorthWest"));
	CreateMesh(FVector(3500.0f, HalfStreet + (SidewalkWidth * 0.5f), CurbHeight * 0.5f),
		FVector(50.0f, SidewalkWidth / 100.0f, CurbHeight / 100.0f), TEXT("Sidewalk_NorthEast"));

	// South Sidewalk
	CreateMesh(FVector(-3500.0f, -(HalfStreet + (SidewalkWidth * 0.5f)), CurbHeight * 0.5f),
		FVector(50.0f, SidewalkWidth / 100.0f, CurbHeight / 100.0f), TEXT("Sidewalk_SouthWest"));
	CreateMesh(FVector(3500.0f, -(HalfStreet + (SidewalkWidth * 0.5f)), CurbHeight * 0.5f),
		FVector(50.0f, SidewalkWidth / 100.0f, CurbHeight / 100.0f), TEXT("Sidewalk_SouthEast"));
}

void ARealWorldCityBlockBuilder::SpawnBuildingsAndInteriors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const float HalfStreet = StreetWidth * 0.5f;
	const float BuildingBaseY = HalfStreet + SidewalkWidth + 650.0f;

	// 1. Modular Commercial Building with Detailed Interior & Acoustic Isolation
	FActorSpawnParameters BuildingParams;
	BuildingParams.Owner = this;
	FVector BuildingLoc = FVector(2000.0f, BuildingBaseY, CurbHeight);
	World->SpawnActor<ARealWorldModularBuilding>(ARealWorldModularBuilding::StaticClass(), BuildingLoc, FRotator::ZeroRotator, BuildingParams);

	// 2. Exterior Office Tower Block (Facade)
	UStaticMeshComponent* TowerFacade = NewObject<UStaticMeshComponent>(this, TEXT("TowerFacadeMesh"));
	if (TowerFacade)
	{
		TowerFacade->RegisterComponent();
		TowerFacade->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);
		TowerFacade->SetRelativeLocation(FVector(-3200.0f, BuildingBaseY + 600.0f, 2000.0f));
		TowerFacade->SetRelativeScale3D(FVector(36.0f, 24.0f, 40.0f)); // 36m x 24m x 40m
		TowerFacade->SetCollisionProfileName(TEXT("BlockAll"));
	}
}

void ARealWorldCityBlockBuilder::SpawnVaultTestObstacles()
{
	auto CreateObstacle = [this](const FVector& Pos, const FVector& Scale, const TCHAR* Name)
	{
		UStaticMeshComponent* Obs = NewObject<UStaticMeshComponent>(this, Name);
		if (Obs)
		{
			Obs->RegisterComponent();
			Obs->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);
			Obs->SetRelativeLocation(Pos);
			Obs->SetRelativeScale3D(Scale);
			Obs->SetCollisionProfileName(TEXT("BlockAll"));
		}
	};

	// 1. Low Concrete Barrier: 55cm high (Low Vault)
	CreateObstacle(FVector(600.0f, 850.0f, 27.5f), FVector(0.5f, 3.5f, 0.55f), TEXT("Vault_LowBarrier_55cm"));

	// 2. Medium Waist Wall: 95cm high (Waist Vault)
	CreateObstacle(FVector(1100.0f, 850.0f, 47.5f), FVector(0.5f, 3.5f, 0.95f), TEXT("Vault_MediumWall_95cm"));

	// 3. High Ledge Mantle: 180cm high (High Mantle)
	CreateObstacle(FVector(1600.0f, 850.0f, 90.0f), FVector(1.0f, 3.5f, 1.80f), TEXT("Vault_HighLedge_180cm"));
}

void ARealWorldCityBlockBuilder::SpawnStreetFurniture()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const float NorthPropY = (StreetWidth * 0.5f) + 120.0f;
	const float SouthPropY = -NorthPropY;

	auto SpawnProp = [World, this](EStreetPropType Type, const FVector& Pos, const FRotator& Rot)
	{
		FActorSpawnParameters P;
		P.Owner = this;
		ARealWorldStreetProp* Prop = World->SpawnActor<ARealWorldStreetProp>(ARealWorldStreetProp::StaticClass(), Pos, Rot, P);
		if (Prop)
		{
			Prop->PropType = Type;
			Prop->SetupPropDimensions();
		}
	};

	// Streetlights
	SpawnProp(EStreetPropType::StreetLight, FVector(-2200.0f, NorthPropY, CurbHeight), FRotator(0.0f, 0.0f, 0.0f));
	SpawnProp(EStreetPropType::StreetLight, FVector(-200.0f, NorthPropY, CurbHeight), FRotator(0.0f, 0.0f, 0.0f));
	SpawnProp(EStreetPropType::StreetLight, FVector(2200.0f, NorthPropY, CurbHeight), FRotator(0.0f, 0.0f, 0.0f));
	SpawnProp(EStreetPropType::StreetLight, FVector(-2200.0f, SouthPropY, CurbHeight), FRotator(0.0f, 180.0f, 0.0f));
	SpawnProp(EStreetPropType::StreetLight, FVector(2200.0f, SouthPropY, CurbHeight), FRotator(0.0f, 180.0f, 0.0f));

	// Benches
	SpawnProp(EStreetPropType::Bench, FVector(-1600.0f, NorthPropY + 70.0f, CurbHeight), FRotator(0.0f, 90.0f, 0.0f));
	SpawnProp(EStreetPropType::Bench, FVector(1200.0f, NorthPropY + 70.0f, CurbHeight), FRotator(0.0f, 90.0f, 0.0f));

	// Trash Bins
	SpawnProp(EStreetPropType::TrashBin, FVector(-1400.0f, NorthPropY, CurbHeight), FRotator::ZeroRotator);
	SpawnProp(EStreetPropType::TrashBin, FVector(1400.0f, NorthPropY, CurbHeight), FRotator::ZeroRotator);

	// Fire Hydrants
	SpawnProp(EStreetPropType::FireHydrant, FVector(-600.0f, NorthPropY - 20.0f, CurbHeight), FRotator::ZeroRotator);
	SpawnProp(EStreetPropType::FireHydrant, FVector(600.0f, SouthPropY + 20.0f, CurbHeight), FRotator(0.0f, 180.0f, 0.0f));

	// Parking Meters
	SpawnProp(EStreetPropType::ParkingMeter, FVector(-2800.0f, NorthPropY, CurbHeight), FRotator::ZeroRotator);
	SpawnProp(EStreetPropType::ParkingMeter, FVector(-2400.0f, NorthPropY, CurbHeight), FRotator::ZeroRotator);

	// Dumpster in commercial alley
	SpawnProp(EStreetPropType::Dumpster, FVector(3000.0f, NorthPropY + 350.0f, CurbHeight), FRotator(0.0f, 90.0f, 0.0f));

	// Planters
	SpawnProp(EStreetPropType::Planter, FVector(-1200.0f, NorthPropY + 50.0f, CurbHeight), FRotator::ZeroRotator);
	SpawnProp(EStreetPropType::Planter, FVector(400.0f, NorthPropY + 50.0f, CurbHeight), FRotator::ZeroRotator);

	// Electrical Utility Box
	SpawnProp(EStreetPropType::UtilityBox, FVector(-3500.0f, NorthPropY + 60.0f, CurbHeight), FRotator::ZeroRotator);
}

void ARealWorldCityBlockBuilder::SpawnPCGAndAtmosphere()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters P;
	P.Owner = this;

	// 1. Atmosphere Controller (Physical Sun/Moon/Sky/Fog/Storm)
	World->SpawnActor<ARealWorldAtmosphereController>(ARealWorldAtmosphereController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, P);

	// 2. PCG Environment Dressing (Deterministic seam grass & gutter clutter)
	ARealWorldPCGEnvironment* PCG = World->SpawnActor<ARealWorldPCGEnvironment>(ARealWorldPCGEnvironment::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, P);
	if (PCG)
	{
		// Add exclusion zone over road surface and storefront doorway
		FPCGExclusionZone RoadZone;
		RoadZone.BoxBounds = FBox(FVector(-6000.0f, -700.0f, 0.0f), FVector(6000.0f, 700.0f, 100.0f));
		PCG->ExclusionZones.Add(RoadZone);

		FPCGExclusionZone StoreDoorZone;
		StoreDoorZone.BoxBounds = FBox(FVector(1850.0f, 900.0f, 0.0f), FVector(2150.0f, 1200.0f, 250.0f));
		PCG->ExclusionZones.Add(StoreDoorZone);

		PCG->GenerateEnvironmentDressing();
	}
}

void ARealWorldCityBlockBuilder::SpawnActorsAndNPCs()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const float NorthPropY = (StreetWidth * 0.5f) + 120.0f;

	FActorSpawnParameters P;
	P.Owner = this;

	// 1. Validated Player Start on the sidewalk
	FVector StartLoc = FVector(-3000.0f, NorthPropY + 30.0f, CurbHeight + 10.0f);
	World->SpawnActor<ARealWorldPlayerStart>(ARealWorldPlayerStart::StaticClass(), StartLoc, FRotator(0.0f, 0.0f, 0.0f), P);

	// 2. City Block Crowd Manager (Governs 24 Diverse Archetypes & Store Shopping Routines)
	World->SpawnActor<ARealWorldCrowdManager>(ARealWorldCrowdManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, P);

	// 3. Cash Pickup inside commercial interior
	const float BuildingBaseY = (StreetWidth * 0.5f) + SidewalkWidth + 650.0f;
	FVector CashLoc = FVector(2000.0f, BuildingBaseY - 200.0f, CurbHeight + 45.0f);
	World->SpawnActor<ARealWorldPickup>(ARealWorldPickup::StaticClass(), CashLoc, FRotator::ZeroRotator, P);
}
