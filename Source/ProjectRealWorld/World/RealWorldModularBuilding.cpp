#include "World/RealWorldModularBuilding.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/BoxComponent.h"
#include "Interaction/RealWorldDoor.h"
#include "World/RealWorldSpawnerSubsystem.h"
#include "Engine/World.h"

ARealWorldModularBuilding::ARealWorldModularBuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BuildingRoot"));
	SetRootComponent(SceneRoot);

	UpperFloorCount = 3;

	InteriorThresholdTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorThresholdTrigger"));
	InteriorThresholdTrigger->SetupAttachment(SceneRoot);
	InteriorThresholdTrigger->SetBoxExtent(FVector(120.0f, 150.0f, 160.0f));
	InteriorThresholdTrigger->SetRelativeLocation(FVector(0.0f, -650.0f, 160.0f));
	InteriorThresholdTrigger->SetCollisionProfileName(TEXT("Trigger"));
}

void ARealWorldModularBuilding::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	AssembleModularBuilding();
}

void ARealWorldModularBuilding::BeginPlay()
{
	Super::BeginPlay();

	if (InteriorThresholdTrigger)
	{
		InteriorThresholdTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARealWorldModularBuilding::OnInteriorTriggerBeginOverlap);
		InteriorThresholdTrigger->OnComponentEndOverlap.AddDynamic(this, &ARealWorldModularBuilding::OnInteriorTriggerEndOverlap);
	}
}

void ARealWorldModularBuilding::OnInteriorTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && Pawn->IsLocallyControlled())
	{
		if (UWorld* World = GetWorld())
		{
			if (URealWorldSpawnerSubsystem* Spawner = World->GetSubsystem<URealWorldSpawnerSubsystem>())
			{
				Spawner->NotifyInteriorEntered(TEXT("CityConvenienceStore"));
			}
		}
	}
}

void ARealWorldModularBuilding::OnInteriorTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && Pawn->IsLocallyControlled())
	{
		if (UWorld* World = GetWorld())
		{
			if (URealWorldSpawnerSubsystem* Spawner = World->GetSubsystem<URealWorldSpawnerSubsystem>())
			{
				Spawner->NotifyInteriorExited(TEXT("CityConvenienceStore"));
			}
		}
	}
}

void ARealWorldModularBuilding::AssembleModularBuilding()
{
	BuildExteriorShell();
	BuildCommercialInterior();
	BuildRooftopHVAC();
}

void ARealWorldModularBuilding::BuildExteriorShell()
{
	auto CreateMeshPiece = [this](const FVector& Pos, const FVector& Scale, const TCHAR* Name)
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

	// 1. Ground Storefront Left Window & Wall
	CreateMeshPiece(FVector(-500.0f, -650.0f, 220.0f), FVector(6.0f, 0.4f, 4.4f), TEXT("StorefrontLeftWall"));

	// 2. Ground Storefront Right Window & Wall
	CreateMeshPiece(FVector(500.0f, -650.0f, 220.0f), FVector(6.0f, 0.4f, 4.4f), TEXT("StorefrontRightWall"));

	// 3. Storefront Awning / Illuminated Sign Board above entrance
	CreateMeshPiece(FVector(0.0f, -680.0f, 420.0f), FVector(16.0f, 1.2f, 0.8f), TEXT("StorefrontSignBoard"));

	// 4. Upper Floors Modular Facade
	const float FloorHeight = 350.0f;
	const float BaseZ = 450.0f;

	for (int32 i = 0; i < UpperFloorCount; ++i)
	{
		const float CurrentFloorZ = BaseZ + (i * FloorHeight) + (FloorHeight * 0.5f);
		FString FloorName = FString::Printf(TEXT("UpperFloor_%d_Facade"), i + 1);
		CreateMeshPiece(FVector(0.0f, -650.0f, CurrentFloorZ), FVector(16.0f, 0.4f, FloorHeight / 100.0f), *FloorName);
	}

	// 5. Exterior Rear Wall
	const float TotalHeight = BaseZ + (UpperFloorCount * FloorHeight);
	CreateMeshPiece(FVector(0.0f, 650.0f, TotalHeight * 0.5f), FVector(16.0f, 0.4f, TotalHeight / 100.0f), TEXT("RearExteriorWall"));

	// 6. Side Walls
	CreateMeshPiece(FVector(-800.0f, 0.0f, TotalHeight * 0.5f), FVector(0.4f, 13.0f, TotalHeight / 100.0f), TEXT("WestExteriorWall"));
	CreateMeshPiece(FVector(800.0f, 0.0f, TotalHeight * 0.5f), FVector(0.4f, 13.0f, TotalHeight / 100.0f), TEXT("EastExteriorWall"));
}

void ARealWorldModularBuilding::BuildCommercialInterior()
{
	auto CreateInteriorPiece = [this](const FVector& Pos, const FVector& Scale, const TCHAR* Name)
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

	// 1. Tiled Floor
	CreateInteriorPiece(FVector(0.0f, 0.0f, 10.0f), FVector(15.5f, 12.5f, 0.2f), TEXT("InteriorTiledFloor"));

	// 2. Interior Ceiling
	CreateInteriorPiece(FVector(0.0f, 0.0f, 430.0f), FVector(15.5f, 12.5f, 0.3f), TEXT("InteriorCeiling"));

	// 3. Retail Service Counter (3m long, 90cm high)
	CreateInteriorPiece(FVector(-350.0f, -200.0f, 45.0f), FVector(3.0f, 0.8f, 0.9f), TEXT("RetailCheckoutCounter"));

	// 4. Double-Sided Merchandise Gondola Shelves
	CreateInteriorPiece(FVector(200.0f, -100.0f, 90.0f), FVector(0.9f, 6.0f, 1.8f), TEXT("GondolaShelf1"));
	CreateInteriorPiece(FVector(500.0f, -100.0f, 90.0f), FVector(0.9f, 6.0f, 1.8f), TEXT("GondolaShelf2"));

	// 5. Interior Warm Lighting (Recessed Downlights)
	auto CreateDownlight = [this](const FVector& Pos, const TCHAR* Name)
	{
		UPointLightComponent* Light = NewObject<UPointLightComponent>(this, Name);
		if (Light)
		{
			Light->RegisterComponent();
			Light->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);
			Light->SetRelativeLocation(Pos);
			Light->SetIntensity(6500.0f);
			Light->SetAttenuationRadius(750.0f);
			Light->SetLightColor(FLinearColor(1.0f, 0.94f, 0.86f)); // Warm 3200K interior glow
		}
	};

	CreateDownlight(FVector(-300.0f, -200.0f, 390.0f), TEXT("InteriorLight1"));
	CreateDownlight(FVector(200.0f, -100.0f, 390.0f), TEXT("InteriorLight2"));
	CreateDownlight(FVector(500.0f, -100.0f, 390.0f), TEXT("InteriorLight3"));
}

void ARealWorldModularBuilding::BuildRooftopHVAC()
{
	auto CreateHVACPiece = [this](const FVector& Pos, const FVector& Scale, const TCHAR* Name)
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

	const float RoofZ = 450.0f + (UpperFloorCount * 350.0f);

	// Rooftop Parapet wall (1m tall along edge)
	CreateHVACPiece(FVector(0.0f, -650.0f, RoofZ + 50.0f), FVector(16.0f, 0.4f, 1.0f), TEXT("RoofParapetFront"));
	CreateHVACPiece(FVector(0.0f, 650.0f, RoofZ + 50.0f), FVector(16.0f, 0.4f, 1.0f), TEXT("RoofParapetBack"));

	// Rooftop Commercial AC Chillers
	CreateHVACPiece(FVector(-300.0f, 100.0f, RoofZ + 80.0f), FVector(2.0f, 1.5f, 1.6f), TEXT("RooftopChiller1"));
	CreateHVACPiece(FVector(300.0f, 100.0f, RoofZ + 80.0f), FVector(2.0f, 1.5f, 1.6f), TEXT("RooftopChiller2"));
}
