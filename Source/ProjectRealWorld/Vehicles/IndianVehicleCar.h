#pragma once

#include "CoreMinimal.h"
#include "Vehicles/RealWorldVehicleBase.h"
#include "IndianVehicleCar.generated.h"

class USoundCue;

UENUM(BlueprintType)
enum class EIndianCarModel : uint8
{
	Maruti800       UMETA(DisplayName = "Maruti 800 — Iconic Indian Hatchback"),
	SwiftTaxi       UMETA(DisplayName = "Maruti Swift Dzire — Kaali-Peeli / Ola Taxi"),
	InnovaCrysta    UMETA(DisplayName = "Toyota Innova — 7-Seater Family / VIP"),
	PoliceBolero    UMETA(DisplayName = "Mahindra Bolero — Police / VIP Escort")
};

/**
 * AIndianVehicleCar
 * Week 2 Core Deliverable:
 * Indian 4-Wheeled Cars & Commercial Cabs.
 * Features:
 * - Multi-model support (Maruti 800, Swift Taxi, Innova, Police Bolero).
 * - Multi-tone Indian musical horn and police siren.
 * - Roof luggage rack for Indian outstation luggage.
 * - Taxi livery and fare meter integration.
 */
UCLASS()
class PROJECTREALWORLD_API AIndianVehicleCar : public ARealWorldVehicleBase
{
	GENERATED_BODY()

public:
	AIndianVehicleCar();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Model")
	EIndianCarModel CarModel = EIndianCarModel::Maruti800;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Features")
	bool bHasRoofLuggageRack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Features")
	bool bIsTaxi = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Car|Police")
	bool bPoliceSirenActive = false;

	// ─── Audio ───────────────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Audio")
	TObjectPtr<USoundCue> Snd_MusicalHorn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Audio")
	TObjectPtr<USoundCue> Snd_PoliceSiren;

	// ─── Actions ──────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Car|Audio")
	void HonkMusicalHorn();

	UFUNCTION(BlueprintCallable, Category = "Car|Police")
	void TogglePoliceSiren();

protected:
	virtual void BeginPlay() override;
};
