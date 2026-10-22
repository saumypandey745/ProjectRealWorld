// ============================================================================
// RealWorldMotorcycleBase.h
// Project RealWorld — GTA5-class 2-Wheel Motorcycle Physics & Leaning
// Dynamic roll banking, wheelie/stoppie, and high-speed crash ejection ragdoll
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RealWorldMotorcycleBase.generated.h"

class USkeletalMeshComponent;
class UCameraComponent;
class USpringArmComponent;
class ARealWorldCharacter;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRiderEjected);

UCLASS(BlueprintType, Blueprintable)
class PROJECTREALWORLD_API ARealWorldMotorcycleBase : public APawn
{
    GENERATED_BODY()

public:
    ARealWorldMotorcycleBase();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|Mesh")
    TObjectPtr<USkeletalMeshComponent> BikeMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|Camera")
    TObjectPtr<USpringArmComponent> SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    // ─── Motorcycle Specs ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Physics")
    float MaxSpeed = 2400.0f; // ~86 km/h

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Physics")
    float AccelerationRate = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Physics")
    float BrakeStrength = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Physics")
    float TurnRate = 65.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Physics")
    float MaxLeanAngle = 42.0f; // Maximum roll banking degrees in corners

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Physics")
    float LeanInterpSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Physics")
    float WheeliePitchAngle = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Physics")
    float EjectionSpeedThreshold = 750.0f; // cm/s impact speed to eject rider

    // ─── Input & Telemetry ───────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|State")
    float CurrentSpeed = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|State")
    float CurrentLeanRoll = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|State")
    float CurrentWheeliePitch = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|State")
    bool bIsWheelie = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|State")
    TObjectPtr<ARealWorldCharacter> MountedRider;

    // ─── Audio ───────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Audio")
    TObjectPtr<USoundBase> EngineRevSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Audio")
    TObjectPtr<USoundBase> CrashSound;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Motorcycle|Actions")
    void MountRider(ARealWorldCharacter* Rider);

    UFUNCTION(BlueprintCallable, Category = "Motorcycle|Actions")
    void DismountRider();

    UFUNCTION(BlueprintCallable, Category = "Motorcycle|Actions")
    void EjectRider(const FVector& ImpactVelocity);

    UPROPERTY(BlueprintAssignable, Category = "Motorcycle|Events")
    FOnRiderEjected OnRiderEjected;

protected:
    float ThrottleInput = 0.0f;
    float SteeringInput = 0.0f;
    float PitchInput = 0.0f;

    void ApplyDrivePhysics(float DeltaTime);
    void UpdateLeanAndWheelie(float DeltaTime);

    UFUNCTION()
    void OnBikeHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                   UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                   const FHitResult& Hit);
};
