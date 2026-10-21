// ============================================================================
// RealWorldVehicleEntryComponent.h
// Project RealWorld — GTA5-style Vehicle Entry/Exit & Interaction System
// Handles: proximity detection, animated entry/exit, hijack, door physics
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "RealWorldVehicleEntryComponent.generated.h"

class ARealWorldVehicleBase;
class ARealWorldCharacter;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EVehicleEntrySeat : uint8
{
    Driver      UMETA(DisplayName = "Driver"),
    Passenger   UMETA(DisplayName = "Front Passenger"),
    RearLeft    UMETA(DisplayName = "Rear Left"),
    RearRight   UMETA(DisplayName = "Rear Right"),
};

UENUM(BlueprintType)
enum class EVehicleEntryState : uint8
{
    None            UMETA(DisplayName = "None"),
    Approaching     UMETA(DisplayName = "Approaching"),
    EnteringAnim    UMETA(DisplayName = "Entering (Animation)"),
    Seated          UMETA(DisplayName = "Seated"),
    ExitingAnim     UMETA(DisplayName = "Exiting (Animation)"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleEntered, ARealWorldVehicleBase*, Vehicle, EVehicleEntrySeat, Seat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleExited,   ARealWorldVehicleBase*, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleHijack,   ARealWorldCharacter*, Hijacker);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREALWORLD_API URealWorldVehicleEntryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URealWorldVehicleEntryComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                                FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Config ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleEntry|Config")
    float EntryDetectionRadius = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleEntry|Config")
    float EntryAnimationDuration = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleEntry|Config")
    float ExitAnimationDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleEntry|Config")
    FName DriverSocketName = TEXT("VehicleDriverSocket");

    // ─── State ───────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VehicleEntry|State")
    EVehicleEntryState EntryState = EVehicleEntryState::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VehicleEntry|State")
    TObjectPtr<ARealWorldVehicleBase> NearbyVehicle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VehicleEntry|State")
    TObjectPtr<ARealWorldVehicleBase> OccupiedVehicle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VehicleEntry|State")
    EVehicleEntrySeat CurrentSeat = EVehicleEntrySeat::Driver;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "VehicleEntry|Actions")
    bool TryEnterNearbyVehicle(EVehicleEntrySeat PreferredSeat = EVehicleEntrySeat::Driver);

    UFUNCTION(BlueprintCallable, Category = "VehicleEntry|Actions")
    void ExitCurrentVehicle(bool bRollOut = false);

    UFUNCTION(BlueprintCallable, Category = "VehicleEntry|Actions")
    bool IsInVehicle() const { return OccupiedVehicle != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "VehicleEntry|Actions")
    bool CanEnterVehicle() const;

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "VehicleEntry|Events")
    FOnVehicleEntered OnVehicleEntered;

    UPROPERTY(BlueprintAssignable, Category = "VehicleEntry|Events")
    FOnVehicleExited OnVehicleExited;

    UPROPERTY(BlueprintAssignable, Category = "VehicleEntry|Events")
    FOnVehicleHijack OnVehicleHijacked;

    // ─── SFX ─────────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleEntry|SFX")
    TObjectPtr<USoundBase> DoorOpenSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleEntry|SFX")
    TObjectPtr<USoundBase> DoorCloseSound;

private:
    UPROPERTY()
    TObjectPtr<ARealWorldCharacter> OwnerCharacter;

    float EntryAnimTimer = 0.0f;
    FVector ExitLocation = FVector::ZeroVector;

    void TickProximityDetection();
    void TickEntryAnimation(float DeltaTime);
    void TickExitAnimation(float DeltaTime);

    void BeginEntryAnimation(ARealWorldVehicleBase* Vehicle, EVehicleEntrySeat Seat);
    void CompleteEntry();
    void BeginExitAnimation(bool bRollOut);
    void CompleteExit();

    ARealWorldVehicleBase* FindNearestVehicle() const;
    FName GetSeatSocketName(EVehicleEntrySeat Seat) const;
};
