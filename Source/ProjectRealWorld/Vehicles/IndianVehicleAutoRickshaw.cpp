#include "Vehicles/IndianVehicleAutoRickshaw.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AIndianVehicleAutoRickshaw::AIndianVehicleAutoRickshaw()
{
	Category = EVehicleType::SUV; // Mapped for lightweight high-torque profile
	MaxForwardSpeedKmH = 65.0f;    // Real auto max speed ~60-65 km/h
	FuelCapacity = 8.0f;           // 8-litre CNG/petrol tank
	CurrentFuel = 8.0f;
	FuelConsumptionRate = 0.002f;
	MaxVehicleHealth = 650.0f;     // Lightweight sheet metal
	CurrentVehicleHealth = 650.0f;
}

void AIndianVehicleAutoRickshaw::BeginPlay()
{
	Super::BeginPlay();
	LastFrameLocation = GetActorLocation();
}

void AIndianVehicleAutoRickshaw::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update Rickshaw Fare Meter if active
	if (bMeterDown && bEngineRunning)
	{
		const FVector CurrentLoc = GetActorLocation();
		const float StepDist = FVector::Dist2D(CurrentLoc, LastFrameLocation);
		LastFrameLocation = CurrentLoc;

		TripDistanceMeters += StepDist;

		// Calculate fare: Base fare for first 1.5 km, then RatePerKm
		const float DistanceKm = TripDistanceMeters / 100000.0f; // UE units (100 = 1m)
		if (DistanceKm > 1.5f)
		{
			CurrentFareAmount = BaseFareINR + ((DistanceKm - 1.5f) * RatePerKmINR);
		}
		else
		{
			CurrentFareAmount = BaseFareINR;
		}
	}
	else
	{
		LastFrameLocation = GetActorLocation();
	}
}

void AIndianVehicleAutoRickshaw::HonkIndianHorn()
{
	if (Snd_AutoHorn)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Snd_AutoHorn, GetActorLocation(), 1.0f, FMath::RandRange(0.95f, 1.05f));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[AutoRickshaw] Ponk-Ponk! Horn Honked at %s"), *GetActorLocation().ToString());
	}
}

void AIndianVehicleAutoRickshaw::StartMeterTrip()
{
	bMeterDown = true;
	TripDistanceMeters = 0.0f;
	CurrentFareAmount = BaseFareINR;
	LastFrameLocation = GetActorLocation();
	UE_LOG(LogTemp, Log, TEXT("[AutoRickshaw] Meter Down. Base Fare: Rs %.2f"), BaseFareINR);
}

float AIndianVehicleAutoRickshaw::EndMeterTrip()
{
	bMeterDown = false;
	const float FinalFare = CurrentFareAmount;
	UE_LOG(LogTemp, Log, TEXT("[AutoRickshaw] Meter Up. Final Trip Fare: Rs %.2f (Dist: %.2f km)"),
		FinalFare, TripDistanceMeters / 100000.0f);
	return FinalFare;
}

FString AIndianVehicleAutoRickshaw::GetFormattedFareString() const
{
	return FString::Printf(TEXT("₹ %.1f"), CurrentFareAmount);
}
