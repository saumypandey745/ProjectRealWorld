#pragma once

#include "CoreMinimal.h"
#include "Vehicles/RealWorldVehicleBase.h"
#include "IndianVehicleMotorcycle.generated.h"

class USoundCue;

UENUM(BlueprintType)
enum class EIndianBikeModel : uint8
{
	HeroSplendor    UMETA(DisplayName = "Hero Splendor 100cc — Common Commuter"),
	BajajPulsar     UMETA(DisplayName = "Bajaj Pulsar 150/220 — Street Sport"),
	RoyalBullet     UMETA(DisplayName = "Royal Enfield Classic 350 — Thump Bullet"),
	HondaActiva     UMETA(DisplayName = "Honda Activa 6G — Scooter")
};

/**
 * AIndianVehicleMotorcycle
 * Week 2 Core Deliverable:
 * 2-Wheeled Indian Commuter & Cruiser Motorcycle.
 * Features:
 * - Dynamic lean angle during high-speed cornering.
 * - Helmet requirement (police fine/challan mechanic if riding without helmet).
 * - Kickstart mechanic & authentic engine thump audio.
 * - Pillion passenger seat for companion/taxi rides.
 * - Mobile responsive touch steering and wheelie stunt support.
 */
UCLASS()
class PROJECTREALWORLD_API AIndianVehicleMotorcycle : public ARealWorldVehicleBase
{
	GENERATED_BODY()

public:
	AIndianVehicleMotorcycle();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Model")
	EIndianBikeModel BikeModel = EIndianBikeModel::HeroSplendor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|Physics")
	float CurrentLeanAngle = 0.0f; // Degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Physics")
	float MaxLeanAngle = 35.0f; // Max motorcycle banking angle

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Helmet")
	bool bPlayerHasHelmet = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|Stunt")
	bool bIsDoingWheelie = false;

	// ─── Audio ───────────────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Audio")
	TObjectPtr<USoundCue> Snd_BikeHorn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Audio")
	TObjectPtr<USoundCue> Snd_Kickstart;

	// ─── Actions ──────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Actions")
	void HonkIndianBikeHorn();

	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Actions")
	void ToggleHelmet();

	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Stunt")
	void TriggerWheelie(bool bActive);

protected:
	virtual void BeginPlay() override;

private:
	void UpdateLeanAngle(float DeltaTime);
};
