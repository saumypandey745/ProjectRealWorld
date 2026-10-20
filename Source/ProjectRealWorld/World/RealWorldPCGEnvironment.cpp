#include "World/RealWorldPCGEnvironment.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Math/RandomStream.h"

ARealWorldPCGEnvironment::ARealWorldPCGEnvironment()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("PCGRoot"));
	SetRootComponent(RootComp);

	SidewalkGrassInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SidewalkGrassInstances"));
	SidewalkGrassInstances->SetupAttachment(RootComp);
	SidewalkGrassInstances->SetCollisionProfileName(TEXT("NoCollision"));

	StreetClutterInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("StreetClutterInstances"));
	StreetClutterInstances->SetupAttachment(RootComp);
	StreetClutterInstances->SetCollisionProfileName(TEXT("NoCollision"));

	PlanterVegetationInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PlanterVegetationInstances"));
	PlanterVegetationInstances->SetupAttachment(RootComp);
	PlanterVegetationInstances->SetCollisionProfileName(TEXT("NoCollision"));

	GenerationSeed = 1337;
	GrassTuftBudget = 120;
	ClutterBudget = 45;
}

void ARealWorldPCGEnvironment::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GenerateEnvironmentDressing();
}

bool ARealWorldPCGEnvironment::IsInExclusionZone(const FVector& Location) const
{
	for (const FPCGExclusionZone& Zone : ExclusionZones)
	{
		if (Zone.BoxBounds.IsInsideOrOn(Location))
		{
			return true;
		}
	}
	return false;
}

void ARealWorldPCGEnvironment::GenerateEnvironmentDressing()
{
	if (!SidewalkGrassInstances || !StreetClutterInstances || !PlanterVegetationInstances) return;

	SidewalkGrassInstances->ClearInstances();
	StreetClutterInstances->ClearInstances();
	PlanterVegetationInstances->ClearInstances();

	FRandomStream Rng(GenerationSeed);

	// 1. Sidewalk Seam Grass Tufts (spawning along curb edge lines at Y = 710 and Y = -710)
	for (int32 i = 0; i < GrassTuftBudget; ++i)
	{
		const float Side = (Rng.FRand() > 0.5f) ? 1.0f : -1.0f;
		const float PosX = Rng.FRandRange(-5500.0f, 5500.0f);
		const float PosY = (Side * 710.0f) + Rng.FRandRange(-25.0f, 25.0f);
		const float PosZ = 16.0f; // Curb top level

		const FVector Candidate(PosX, PosY, PosZ);
		if (!IsInExclusionZone(Candidate))
		{
			const FRotator Rot(0.0f, Rng.FRandRange(0.0f, 360.0f), 0.0f);
			const float ScaleFactor = Rng.FRandRange(0.6f, 1.25f);
			FTransform InstanceTransform(Rot, Candidate, FVector(ScaleFactor, ScaleFactor, ScaleFactor));
			SidewalkGrassInstances->AddInstance(InstanceTransform, true);
		}
	}

	// 2. Street Clutter & Debris (Paper cups, fallen autumn leaves, cartons in gutters)
	for (int32 i = 0; i < ClutterBudget; ++i)
	{
		const float Side = (Rng.FRand() > 0.5f) ? 1.0f : -1.0f;
		const float PosX = Rng.FRandRange(-5000.0f, 5000.0f);
		const float PosY = (Side * 690.0f) + Rng.FRandRange(-30.0f, 150.0f); // Gutters & lower sidewalk
		const float PosZ = (FMath::Abs(PosY) > 700.0f) ? 16.0f : 2.0f;

		const FVector Candidate(PosX, PosY, PosZ);
		if (!IsInExclusionZone(Candidate))
		{
			const FRotator Rot(Rng.FRandRange(-5.0f, 5.0f), Rng.FRandRange(0.0f, 360.0f), Rng.FRandRange(-5.0f, 5.0f));
			const float ScaleFactor = Rng.FRandRange(0.8f, 1.15f);
			FTransform InstanceTransform(Rot, Candidate, FVector(ScaleFactor, ScaleFactor, ScaleFactor));
			StreetClutterInstances->AddInstance(InstanceTransform, true);
		}
	}

	// 3. Planter Shrubbery (Vegetation in sidewalk planters)
	TArray<FVector> PlanterCoords = {
		FVector(-1200.0f, 850.0f, 75.0f),
		FVector(400.0f, 850.0f, 75.0f),
		FVector(-1200.0f, -850.0f, 75.0f),
		FVector(400.0f, -850.0f, 75.0f)
	};

	for (const FVector& BaseCoord : PlanterCoords)
	{
		const FRotator Rot(0.0f, Rng.FRandRange(0.0f, 360.0f), 0.0f);
		FTransform InstanceTransform(Rot, BaseCoord, FVector(1.0f, 1.0f, 1.0f));
		PlanterVegetationInstances->AddInstance(InstanceTransform, true);
	}
}
