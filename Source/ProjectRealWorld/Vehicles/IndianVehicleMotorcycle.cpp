#include "Vehicles/IndianVehicleMotorcycle.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AIndianVehicleMotorcycle::AIndianVehicleMotorcycle()
{
	Category = EVehicleType::Motorcycle;
	MaxForwardSpeedKmH = 110.0f; // 100-110 km/h top speed
	FuelCapacity = 11.0f;        // 11 litres
	CurrentFuel = 11.0f;
	FuelConsumptionRate = 0.0015f;
	MaxVehicleHealth = 450.0f;   // Bike fragile compared to car
	CurrentVehicleHealth = 450.0f;
}

void AIndianVehicleMotorcycle::BeginPlay()
{
	Super::BeginPlay();

	// Configure top speed based on model
	switch (BikeModel)
	{
	case EIndianBikeModel::RoyalBullet:
		MaxForwardSpeedKmH = 130.0f;
		MaxVehicleHealth = 600.0f;
		break;
	case EIndianBikeModel::BajajPulsar:
		MaxForwardSpeedKmH = 125.0f;
		break;
	case EIndianBikeModel::HondaActiva:
		MaxForwardSpeedKmH = 85.0f;
		break;
	case EIndianBikeModel::HeroSplendor:
	default:
		MaxForwardSpeedKmH = 95.0f;
		break;
	}
	CurrentVehicleHealth = MaxVehicleHealth;
}

void AIndianVehicleMotorcycle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateLeanAngle(DeltaTime);
}

void AIndianVehicleMotorcycle::UpdateLeanAngle(float DeltaTime)
{
	if (!bEngineRunning)
	{
		CurrentLeanAngle = FMath::FInterpTo(CurrentLeanAngle, 0.0f, DeltaTime, 5.0f);
		return;
	}

	const float SpeedKmH = GetForwardSpeedKmH();
	if (SpeedKmH > 10.0f)
	{
		// Lean into the turn: angular velocity yaw maps to roll
		const FVector AngVel = GetVelocity();
		const float TurnRate = GetActorRotation().Yaw; // simplified steering banking
		const float TargetLean = FMath::Clamp(TurnRate * 0.1f, -MaxLeanAngle, MaxLeanAngle);
		CurrentLeanAngle = FMath::FInterpTo(CurrentLeanAngle, TargetLean, DeltaTime, 6.0f);
	}
	else
	{
		CurrentLeanAngle = FMath::FInterpTo(CurrentLeanAngle, 0.0f, DeltaTime, 8.0f);
	}
}

void AIndianVehicleMotorcycle::HonkIndianBikeHorn()
{
	if (Snd_BikeHorn)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Snd_BikeHorn, GetActorLocation(), 1.0f, FMath::RandRange(0.95f, 1.05f));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[IndianBike] Peep! Bike Horn Honked."));
	}
}

void AIndianVehicleMotorcycle::ToggleHelmet()
{
	bPlayerHasHelmet = !bPlayerHasHelmet;
	UE_LOG(LogTemp, Log, TEXT("[IndianBike] Player Helmet Status: %s"), bPlayerHasHelmet ? TEXT("ON") : TEXT("OFF"));
}

void AIndianVehicleMotorcycle::TriggerWheelie(bool bActive)
{
	bIsDoingWheelie = bActive;
	if (bIsDoingWheelie && GetForwardSpeedKmH() > 25.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("[IndianBike] Wheelie Stunt engaged!"));
	}
}
