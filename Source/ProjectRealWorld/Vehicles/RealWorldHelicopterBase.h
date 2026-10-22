// ============================================================================
// RealWorldHelicopterBase.h
// Project RealWorld — GTA V-Class Attack Helicopter Simulation
// Features: 6-DOF flight physics, collective pitch, rotor blade spooling,
//           dual rapid-fire miniguns, homing missile lock-on, searchlight
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "RealWorldHelicopterBase.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class USpotLightComponent;
class UAudioComponent;
class ARealWorldCharacter;
class ARealWorldProjectileRocket;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHelicopterStateChanged, bool, bIsFlying);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHelicopterTargetLocked, AActor*, Target);

UCLASS(Blueprintable)
class PROJECTREALWORLD_API ARealWorldHelicopterBase : public APawn
{
    GENERATED_BODY()

public:
    ARealWorldHelicopterBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // ─── Visual Components ───────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Helicopter|Components")
    TObjectPtr<USkeletalMeshComponent> HelicopterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Helicopter|Components")
    TObjectPtr<UStaticMeshComponent> MainRotorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Helicopter|Components")
    TObjectPtr<UStaticMeshComponent> TailRotorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Helicopter|Components")
    TObjectPtr<USpotLightComponent> Searchlight;

    // ─── Flight Dynamics Config ──────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Physics")
    float MaxCollectiveLift = 1800.0f;    // Max upward thrust force

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Physics")
    float MaxHorizontalSpeed = 3200.0f;   // Max forward/backward speed (cm/s)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Physics")
    float MaxPitchAngle = 35.0f;          // Forward/back tilt limit (degrees)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Physics")
    float MaxRollAngle = 40.0f;           // Bank turn limit (degrees)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Physics")
    float YawTurnRate = 60.0f;            // Rudder turn rate (deg/s)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Physics")
    float RotorSpoolUpTime = 3.5f;        // Seconds to full RPM before takeoff

    // ─── Weapons Config ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Weapons")
    float MinigunFireRate = 0.08f;        // Seconds per round (dual guns)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Weapons")
    float MinigunDamage = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Weapons")
    float MissileCooldown = 3.0f;         // Seconds between missile volleys

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Weapons")
    float MissileLockRange = 8000.0f;     // Range to lock target (cm)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Weapons")
    TSubclassOf<ARealWorldProjectileRocket> RocketClass;

    // ─── Audio & VFX ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Audio")
    TObjectPtr<USoundBase> EngineRotorSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Audio")
    TObjectPtr<USoundBase> MinigunFireSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Audio")
    TObjectPtr<USoundBase> MissileLaunchSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|Audio")
    TObjectPtr<USoundBase> TargetLockSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Helicopter|VFX")
    TObjectPtr<UNiagaraSystem> RotorDustDownwashFX;

    // ─── Flight Controls (Mobile Touch / Gamepad Callbacks) ──────────────────
    UFUNCTION(BlueprintCallable, Category = "Helicopter|Controls")
    void SetCollectiveInput(float Value); // +1 = Up, -1 = Down

    UFUNCTION(BlueprintCallable, Category = "Helicopter|Controls")
    void SetPitchInput(float Value);      // +1 = Forward, -1 = Backward

    UFUNCTION(BlueprintCallable, Category = "Helicopter|Controls")
    void SetRollInput(float Value);       // +1 = Bank Right, -1 = Bank Left

    UFUNCTION(BlueprintCallable, Category = "Helicopter|Controls")
    void SetYawInput(float Value);        // +1 = Turn Right, -1 = Turn Left

    // ─── Weapons Actions ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Helicopter|Weapons")
    void StartFiringMiniguns();

    UFUNCTION(BlueprintCallable, Category = "Helicopter|Weapons")
    void StopFiringMiniguns();

    UFUNCTION(BlueprintCallable, Category = "Helicopter|Weapons")
    void FireMissileVolley();

    // ─── Entry / Exit ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Helicopter|Actions")
    bool MountPilot(ARealWorldCharacter* Pilot);

    UFUNCTION(BlueprintCallable, Category = "Helicopter|Actions")
    void DismountPilot();

    UFUNCTION(BlueprintPure, Category = "Helicopter|State")
    bool IsEngineSpooling() const { return bEngineStarted; }

    UFUNCTION(BlueprintPure, Category = "Helicopter|State")
    float GetRotorRPMPercent() const { return CurrentRotorRPM / 1000.0f; }

    // ─── Events ──────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Helicopter|Events")
    FOnHelicopterStateChanged OnHelicopterStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Helicopter|Events")
    FOnHelicopterTargetLocked OnHelicopterTargetLocked;

private:
    // Input state
    float CollectiveInput = 0.0f;
    float PitchInput = 0.0f;
    float RollInput = 0.0f;
    float YawInput = 0.0f;

    // Flight state
    bool bEngineStarted = false;
    float CurrentRotorRPM = 0.0f;
    float TargetPitch = 0.0f;
    float TargetRoll = 0.0f;
    FVector CurrentVelocity = FVector::ZeroVector;

    // Pilot reference
    UPROPERTY()
    TObjectPtr<ARealWorldCharacter> CurrentPilot;

    // Weapons state
    bool bFiringMiniguns = false;
    float MinigunCooldownTimer = 0.0f;
    float MissileCooldownTimer = 0.0f;
    bool bNextGunLeft = false;

    UPROPERTY()
    TObjectPtr<AActor> CurrentLockedTarget;

    UPROPERTY()
    TObjectPtr<UAudioComponent> RotorAudioComp;

    void UpdateFlightPhysics(float DeltaTime);
    void UpdateRotorAnimation(float DeltaTime);
    void UpdateWeapons(float DeltaTime);
    void FireMinigunBullet();
    void SearchTargetLock();
};
