// ============================================================================
// RealWorldBoatBase.h
// Project RealWorld — GTA V-Class Speedboat & Watercraft Simulation
// Features: Water surface buoyancy, propeller thrust, wave damping,
//           dynamic wake VFX, driver mounting/dismounting
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "RealWorldBoatBase.generated.h"

class UStaticMeshComponent;
class UAudioComponent;
class ARealWorldCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoatDismounted, ARealWorldCharacter*, Driver);

UCLASS(Blueprintable)
class PROJECTREALWORLD_API ARealWorldBoatBase : public APawn
{
    GENERATED_BODY()

public:
    ARealWorldBoatBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // ─── Visual Components ───────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boat|Components")
    TObjectPtr<UStaticMeshComponent> BoatMesh;

    // ─── Buoyancy & Physics Config ───────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|Physics")
    float WaterSurfaceZ = 0.0f;           // World Z level of the ocean surface

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|Physics")
    float MaxForwardSpeed = 3800.0f;      // Top speed in water (cm/s)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|Physics")
    float ReverseSpeed = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|Physics")
    float TurnRate = 55.0f;               // Rudder turn rate (deg/s)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|Physics")
    float MaxBankAngle = 22.0f;           // Roll angle when turning hard

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|Physics")
    float BuoyancyStrength = 12.0f;       // Vertical spring restoration force

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|Physics")
    float WaveBobAmplitude = 18.0f;       // Idle wave pitch/bob height

    // ─── VFX / SFX ───────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|Audio")
    TObjectPtr<USoundBase> EngineWaterSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|Audio")
    TObjectPtr<USoundBase> WaterSplashSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|VFX")
    TObjectPtr<UNiagaraSystem> WaterWakeFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|VFX")
    TObjectPtr<UNiagaraSystem> BowSprayFX;

    // ─── Controls ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Boat|Controls")
    void SetThrottleInput(float Value); // +1 = Forward, -1 = Reverse

    UFUNCTION(BlueprintCallable, Category = "Boat|Controls")
    void SetSteeringInput(float Value); // -1 = Left, +1 = Right

    // ─── Interaction ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Boat|Actions")
    bool MountDriver(ARealWorldCharacter* Driver);

    UFUNCTION(BlueprintCallable, Category = "Boat|Actions")
    void DismountDriver();

    UFUNCTION(BlueprintPure, Category = "Boat|State")
    bool IsInWater() const { return bSubmergedInWater; }

    UFUNCTION(BlueprintPure, Category = "Boat|State")
    float GetCurrentSpeedKmh() const { return CurrentForwardSpeed * 0.036f; }

    UPROPERTY(BlueprintAssignable, Category = "Boat|Events")
    FOnBoatDismounted OnBoatDismounted;

private:
    float ThrottleInput = 0.0f;
    float SteeringInput = 0.0f;

    float CurrentForwardSpeed = 0.0f;
    float CurrentBankAngle = 0.0f;
    float WaveTimer = 0.0f;
    bool bSubmergedInWater = false;

    UPROPERTY()
    TObjectPtr<ARealWorldCharacter> CurrentDriver;

    UPROPERTY()
    TObjectPtr<UAudioComponent> EngineAudioComp;

    void SimulateWaterPhysics(float DeltaTime);
    void UpdateAudioVFX(float DeltaTime);
};
