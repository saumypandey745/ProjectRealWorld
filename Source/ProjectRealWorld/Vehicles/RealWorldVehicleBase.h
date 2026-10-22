#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Interaction/InteractableInterface.h"
#include "InputActionValue.h"
#include "RealWorldVehicleBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class ARealWorldCharacter;
class UInputMappingContext;
class UInputAction;

UENUM(BlueprintType)
enum class EVehicleType : uint8
{
	Sedan UMETA(DisplayName = "Sedan"),
	SUV UMETA(DisplayName = "SUV"),
	SportsCar UMETA(DisplayName = "Sports Car"),
	Truck UMETA(DisplayName = "Truck"),
	Motorcycle UMETA(DisplayName = "Motorcycle"),
	PoliceCruiser UMETA(DisplayName = "Police Cruiser"),
	Ambulance UMETA(DisplayName = "Ambulance")
};

UENUM(BlueprintType)
enum class EVehicleDamageState : uint8
{
	Pristine UMETA(DisplayName = "Pristine"),
	MinorDents UMETA(DisplayName = "Minor Dents"),
	ModerateDamage UMETA(DisplayName = "Moderate Damage"),
	HeavyDamage UMETA(DisplayName = "Heavy Damage"),
	EngineSmoking UMETA(DisplayName = "Engine Smoking"),
	CriticalFire UMETA(DisplayName = "Critical Fire"),
	Totaled UMETA(DisplayName = "Totaled")
};

USTRUCT(BlueprintType)
struct FOwnedVehicleMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ownership")
	FName VehicleVIN;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ownership")
	FString LicensePlateText = TEXT("RW-7892");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ownership")
	FString OwnerPlayerName = TEXT("Protagonist");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ownership")
	bool bIsLocked = false;
};

UCLASS()
class PROJECTREALWORLD_API ARealWorldVehicleBase : public AWheeledVehiclePawn, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ARealWorldVehicleBase();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// --- Camera Architecture ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> VehicleCameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> VehicleCamera;

	// --- Classification & Input ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Type")
	EVehicleType Category = EVehicleType::Sedan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Input")
	TObjectPtr<UInputMappingContext> VehicleInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Input")
	TObjectPtr<UInputAction> IA_ThrottleBrake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Input")
	TObjectPtr<UInputAction> IA_Steering;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Input")
	TObjectPtr<UInputAction> IA_Handbrake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Input")
	TObjectPtr<UInputAction> IA_ExitVehicle;

	// --- Seating & Possession ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|State")
	bool bIsOccupied = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|State")
	TObjectPtr<ARealWorldCharacter> CurrentDriver;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|State")
	TObjectPtr<ARealWorldCharacter> CurrentPassenger;

	// --- Engine & Powertrain State ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Engine")
	bool bEngineRunning = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Engine")
	float EngineRPM = 800.0f; // Idle RPM

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Engine")
	float MaxForwardSpeedKmH = 180.0f;

	// --- Fuel System ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Fuel")
	float FuelCapacity = 65.0f; // Litres

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Fuel")
	float CurrentFuel = 65.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Fuel")
	float FuelConsumptionRate = 0.005f; // Litres per second under throttle

	// --- Health & Damage System ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Damage")
	float MaxVehicleHealth = 1000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Damage")
	float CurrentVehicleHealth = 1000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Damage")
	EVehicleDamageState DamageState = EVehicleDamageState::Pristine;

	// --- Ownership Metadata ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Ownership")
	FOwnedVehicleMetadata OwnershipData;

	// --- Actions ---
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Engine")
	void StartEngine();

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Engine")
	void StopEngine();

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	bool EnterVehicle(ARealWorldCharacter* InDriver);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	bool ExitVehicle();

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Doors")
	void OpenDoor(int32 DoorIndex);

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Doors")
	void CloseDoor(int32 DoorIndex);

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Damage")
	void ApplyPhysicalImpactDamage(float ImpulseMagnitude, FVector ImpactPoint);

	// --- HUD Telemetry Getters ---
	UFUNCTION(BlueprintPure, Category = "Vehicle|Telemetry")
	float GetForwardSpeedKmH() const;

	UFUNCTION(BlueprintPure, Category = "Vehicle|Telemetry")
	float GetFuelPercent() const;

	UFUNCTION(BlueprintPure, Category = "Vehicle|Telemetry")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Vehicle|Telemetry")
	int32 GetCurrentGear() const;

	// --- IInteractableInterface ---
	virtual FText GetInteractionPrompt_Implementation(ARealWorldCharacter* InstigatorCharacter) override;
	virtual bool CanInteract_Implementation(ARealWorldCharacter* InstigatorCharacter) override;
	virtual void Interact_Implementation(ARealWorldCharacter* InstigatorCharacter) override;

protected:
	virtual void BeginPlay() override;

	// Input Callbacks
	void Input_ThrottleBrake(const FInputActionValue& Value);
	void Input_Steer(const FInputActionValue& Value);
	void Input_HandbrakeStarted();
	void Input_HandbrakeStopped();
	void Input_ExitVehicle();

	void UpdateCameraDynamics(float DeltaTime);
	void UpdateFuelAndEngine(float DeltaTime);
	void ReevaluateDamageState();

	UFUNCTION()
	void OnVehicleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	FVector DriverDoorOffset = FVector(-120.0f, 0.0f, 10.0f);
	float TargetArmLength = 520.0f;
};
