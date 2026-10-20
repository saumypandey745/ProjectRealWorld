#include "World/RealWorldStreetProps.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"

ARealWorldStreetProp::ARealWorldStreetProp()
{
	PrimaryActorTick.bCanEverTick = false;

	PropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PropMesh"));
	SetRootComponent(PropMesh);
	PropMesh->SetCollisionProfileName(TEXT("BlockAll"));

	LightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("LightComponent"));
	LightComponent->SetupAttachment(PropMesh);
	LightComponent->SetVisibility(false);
	LightComponent->SetIntensity(0.0f);
}

void ARealWorldStreetProp::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetupPropDimensions();
}

void ARealWorldStreetProp::SetupPropDimensions()
{
	if (!PropMesh) return;

	switch (PropType)
	{
	case EStreetPropType::StreetLight:
		PropMesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 6.5f)); // 6.5m tall
		PropMesh->SetCollisionProfileName(TEXT("BlockAll"));
		if (LightComponent)
		{
			LightComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 620.0f));
			LightComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
			LightComponent->SetIntensity(22000.0f);
			LightComponent->SetAttenuationRadius(1800.0f);
			LightComponent->SetOuterConeAngle(48.0f);
			LightComponent->SetLightColor(FLinearColor(1.0f, 0.92f, 0.78f));
			LightComponent->SetVisibility(true);
		}
		break;

	case EStreetPropType::Bench:
		PropMesh->SetRelativeScale3D(FVector(1.8f, 0.6f, 0.8f)); // 1.8m wide, 80cm tall
		PropMesh->SetCollisionProfileName(TEXT("BlockAll"));
		if (LightComponent) LightComponent->SetVisibility(false);
		break;

	case EStreetPropType::TrashBin:
		PropMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.0f)); // 50x50cm, 1m tall
		PropMesh->SetCollisionProfileName(TEXT("BlockAll"));
		if (LightComponent) LightComponent->SetVisibility(false);
		break;

	case EStreetPropType::Dumpster:
		PropMesh->SetRelativeScale3D(FVector(2.2f, 1.4f, 1.35f)); // 2.2m x 1.4m x 1.35m tall (mantleable)
		PropMesh->SetCollisionProfileName(TEXT("BlockAll"));
		if (LightComponent) LightComponent->SetVisibility(false);
		break;

	case EStreetPropType::FireHydrant:
		PropMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.85f)); // 85cm tall
		PropMesh->SetCollisionProfileName(TEXT("BlockAll"));
		if (LightComponent) LightComponent->SetVisibility(false);
		break;

	case EStreetPropType::ParkingMeter:
		PropMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 1.35f)); // 1.35m tall
		PropMesh->SetCollisionProfileName(TEXT("BlockAll"));
		if (LightComponent) LightComponent->SetVisibility(false);
		break;

	case EStreetPropType::UtilityBox:
		PropMesh->SetRelativeScale3D(FVector(0.9f, 0.6f, 1.2f)); // 1.2m tall
		PropMesh->SetCollisionProfileName(TEXT("BlockAll"));
		if (LightComponent) LightComponent->SetVisibility(false);
		break;

	case EStreetPropType::RoadBarrier:
		PropMesh->SetRelativeScale3D(FVector(2.0f, 0.4f, 0.95f)); // 95cm tall concrete barrier (vaultable)
		PropMesh->SetCollisionProfileName(TEXT("BlockAll"));
		if (LightComponent) LightComponent->SetVisibility(false);
		break;

	case EStreetPropType::TrafficCone:
		PropMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.70f)); // 70cm tall
		// Overlap physics so player or vehicle can kick or displace it
		PropMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		if (LightComponent) LightComponent->SetVisibility(false);
		break;

	case EStreetPropType::Planter:
		PropMesh->SetRelativeScale3D(FVector(1.4f, 1.4f, 0.65f)); // 65cm tall square planter
		PropMesh->SetCollisionProfileName(TEXT("BlockAll"));
		if (LightComponent) LightComponent->SetVisibility(false);
		break;

	case EStreetPropType::ShopSign:
		PropMesh->SetRelativeScale3D(FVector(2.5f, 0.3f, 0.8f));
		PropMesh->SetCollisionProfileName(TEXT("NoCollision"));
		if (LightComponent)
		{
			LightComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			LightComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
			LightComponent->SetIntensity(8000.0f);
			LightComponent->SetAttenuationRadius(600.0f);
			LightComponent->SetLightColor(FLinearColor(1.0f, 0.8f, 0.2f));
			LightComponent->SetVisibility(true);
		}
		break;

	default:
		PropMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		PropMesh->SetCollisionProfileName(TEXT("BlockAll"));
		if (LightComponent) LightComponent->SetVisibility(false);
		break;
	}
}

void ARealWorldStreetProp::SetLightActive(bool bActive)
{
	if (LightComponent)
	{
		LightComponent->SetVisibility(bActive);
	}
}
