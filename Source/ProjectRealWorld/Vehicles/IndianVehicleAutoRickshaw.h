#pragma once

#include "CoreMinimal.h"
#include "Vehicles/RealWorldVehicleBase.h"
#include "World/IndianCityTypes.h"
#include "IndianVehicleAutoRickshaw.generated.h"

class UAudioComponent;
class USoundCue;

/**
 * AIndianVehicleAutoRickshaw
 * Week 2 Core Deliverable:
 * 3-Wheeled Indian Auto Rickshaw (Bajaj RE 4S Compact / Piaggio Ape style).
 * Features:
 * - 3-wheel physics balance (1 front steering fork, 2 rear drive wheels).
 * - Ultra-tight turning radius for narrow Indian bazaar lanes and gali shortcuts.
 * - Interactive Rickshaw Fare Meter (Base Fare Rs 23, Rs 15/km, waiting charge).
 * - Authentic 2-stroke engine audio and "Pee-Ponk" dual squeeze horn.
 * - Passenger pickup & drop gameplay (NPC taxi missions with cash reward).
 * - Canvas roof with Indian slogan decals ("Horn OK Please", "Buri Nazar Wale Tera Munh Kala").
 */
UCLASS()
class PROJECTREALWORLD_API AIndianVehicleAutoRickshaw : public ARealWorldVehicleBase
{
	GENERATED_BODY()

public:
	AIndianVehicleAutoRickshaw();

	virtual void Tick(float DeltaTime) override;

	// ─── Auto Rickshaw Specifications ─────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoRickshaw|Fare")
	float BaseFareINR = 23.0f; // Standard Indian minimum fare

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoRickshaw|Fare")
	float RatePerKmINR = 15.30f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AutoRickshaw|Fare")
	float TripDistanceMeters = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AutoRickshaw|Fare")
	float CurrentFareAmount = 23.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AutoRickshaw|Fare")
	bool bMeterDown = false; // Is fare active

	// ─── Decals & Aesthetics ──────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoRickshaw|Aesthetics")
	FString RearDecalText = TEXT("Horn OK Please");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoRickshaw|Aesthetics")
	FString CityPermitNumber = TEXT("MH-02-AK-4589");

	// ─── Audio ───────────────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoRickshaw|Audio")
	TObjectPtr<USoundCue> Snd_AutoHorn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoRickshaw|Audio")
	TObjectPtr<USoundCue> Snd_AutoEngineTwoStroke;

	// ─── Actions & Mechanics ──────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "AutoRickshaw|Horn")
	void HonkIndianHorn();

	UFUNCTION(BlueprintCallable, Category = "AutoRickshaw|Meter")
	void StartMeterTrip();

	UFUNCTION(BlueprintCallable, Category = "AutoRickshaw|Meter")
	float EndMeterTrip();

	UFUNCTION(BlueprintPure, Category = "AutoRickshaw|Telemetry")
	FString GetFormattedFareString() const;

protected:
	virtual void BeginPlay() override;

private:
	FVector LastFrameLocation = FVector::ZeroVector;
};
