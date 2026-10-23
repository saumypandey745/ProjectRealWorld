#include "Vehicles/IndianVehicleCar.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AIndianVehicleCar::AIndianVehicleCar()
{
	Category = EVehicleType::Sedan;
	MaxForwardSpeedKmH = 140.0f;
	FuelCapacity = 42.0f;
	CurrentFuel = 42.0f;
	FuelConsumptionRate = 0.003f;
	MaxVehicleHealth = 850.0f;
	CurrentVehicleHealth = 850.0f;
}

void AIndianVehicleCar::BeginPlay()
{
	Super::BeginPlay();

	switch (CarModel)
	{
	case EIndianCarModel::Maruti800:
		MaxForwardSpeedKmH = 115.0f;
		FuelCapacity = 28.0f;
		MaxVehicleHealth = 650.0f;
		bIsTaxi = false;
		break;
	case EIndianCarModel::SwiftTaxi:
		MaxForwardSpeedKmH = 150.0f;
		bIsTaxi = true;
		break;
	case EIndianCarModel::InnovaCrysta:
		MaxForwardSpeedKmH = 165.0f;
		FuelCapacity = 55.0f;
		MaxVehicleHealth = 1100.0f;
		break;
	case EIndianCarModel::PoliceBolero:
		MaxForwardSpeedKmH = 135.0f;
		FuelCapacity = 60.0f;
		MaxVehicleHealth = 1300.0f;
		Category = EVehicleType::PoliceCruiser;
		break;
	}

	CurrentFuel = FuelCapacity;
	CurrentVehicleHealth = MaxVehicleHealth;
}

void AIndianVehicleCar::HonkMusicalHorn()
{
	if (Snd_MusicalHorn)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Snd_MusicalHorn, GetActorLocation(), 1.0f, FMath::RandRange(0.98f, 1.02f));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[IndianCar] Po-Po-Poo-Poo! Indian Multi-Tone Horn Honked."));
	}
}

void AIndianVehicleCar::TogglePoliceSiren()
{
	if (CarModel != EIndianCarModel::PoliceBolero) return;

	bPoliceSirenActive = !bPoliceSirenActive;
	if (bPoliceSirenActive && Snd_PoliceSiren)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Snd_PoliceSiren, GetActorLocation());
	}
	UE_LOG(LogTemp, Log, TEXT("[IndianCar] Police Siren Status: %s"), bPoliceSirenActive ? TEXT("ACTIVE") : TEXT("OFF"));
}
