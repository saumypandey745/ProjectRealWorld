// ============================================================================
// RealWorldBoatBase.cpp
// Project RealWorld — GTA V-Class Speedboat Simulation
// ============================================================================

#include "Vehicles/RealWorldBoatBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RealWorldCharacter.h"
#include "Engine/World.h"

ARealWorldBoatBase::ARealWorldBoatBase()
{
    PrimaryActorTick.bCanEverTick = true;

    BoatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoatMesh"));
    BoatMesh->SetSimulatePhysics(false);
    BoatMesh->SetCollisionProfileName(TEXT("Vehicle"));
    SetRootComponent(BoatMesh);
}

void ARealWorldBoatBase::BeginPlay()
{
    Super::BeginPlay();

    if (EngineWaterSound)
    {
        EngineAudioComp = UGameplayStatics::SpawnSoundAttached(
            EngineWaterSound, BoatMesh, NAME_None,
            FVector::ZeroVector, EAttachLocation::KeepRelativeOffset,
            true
        );
        if (EngineAudioComp)
        {
            EngineAudioComp->SetVolumeMultiplier(0.0f);
        }
    }
}

void ARealWorldBoatBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ARealWorldBoatBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    SimulateWaterPhysics(DeltaTime);
    UpdateAudioVFX(DeltaTime);
}

void ARealWorldBoatBase::SimulateWaterPhysics(float DeltaTime)
{
    WaveTimer += DeltaTime * 2.0f;
    const FVector CurrentLoc = GetActorLocation();

    // 1. Water surface height with natural ocean swell
    const float WaveOffset = FMath::Sin(WaveTimer) * WaveBobAmplitude;
    const float TargetZ = WaterSurfaceZ + WaveOffset;

    bSubmergedInWater = (CurrentLoc.Z <= WaterSurfaceZ + 50.0f);

    // Vertical buoyancy restoration
    float NewZ = CurrentLoc.Z;
    if (bSubmergedInWater)
    {
        NewZ = FMath::FInterpTo(CurrentLoc.Z, TargetZ, DeltaTime, BuoyancyStrength);
    }
    else
    {
        // Gravity fall back to water surface
        NewZ = FMath::Max(CurrentLoc.Z - (980.0f * DeltaTime), TargetZ);
    }

    // 2. Throttle & Speed
    const float TargetSpeed = (ThrottleInput >= 0.0f) ? (ThrottleInput * MaxForwardSpeed)
                                                      : (ThrottleInput * ReverseSpeed);
    CurrentForwardSpeed = FMath::FInterpTo(CurrentForwardSpeed, TargetSpeed, DeltaTime, 2.0f);

    // 3. Steering & Banking
    const float SpeedRatio = FMath::Abs(CurrentForwardSpeed) / MaxForwardSpeed;
    const float DeltaYaw = SteeringInput * TurnRate * SpeedRatio * DeltaTime;

    FRotator CurrentRot = GetActorRotation();
    CurrentRot.Yaw += DeltaYaw;

    // Dynamic bank roll when steering at speed
    const float TargetBank = SteeringInput * MaxBankAngle * SpeedRatio;
    CurrentBankAngle = FMath::FInterpTo(CurrentBankAngle, TargetBank, DeltaTime, 4.0f);
    CurrentRot.Roll = CurrentBankAngle;

    // Pitch: Bow lifts up when accelerating
    const float AccelerationPitch = (ThrottleInput > 0.0f) ? (SpeedRatio * 8.0f) : 0.0f;
    CurrentRot.Pitch = -AccelerationPitch + (FMath::Cos(WaveTimer * 1.5f) * 3.0f);

    // Apply movement
    const FVector ForwardMovement = GetActorForwardVector() * CurrentForwardSpeed * DeltaTime;
    FVector FinalLoc = CurrentLoc + ForwardMovement;
    FinalLoc.Z = NewZ;

    FHitResult Hit;
    SetActorLocationAndRotation(FinalLoc, CurrentRot, true, &Hit);
}

void ARealWorldBoatBase::UpdateAudioVFX(float DeltaTime)
{
    if (EngineAudioComp)
    {
        const float SpeedNorm = FMath::Clamp(FMath::Abs(CurrentForwardSpeed) / MaxForwardSpeed, 0.0f, 1.0f);
        const float TargetVol = CurrentDriver ? FMath::Lerp(0.3f, 1.0f, SpeedNorm) : 0.0f;
        EngineAudioComp->SetVolumeMultiplier(TargetVol);
        EngineAudioComp->SetPitchMultiplier(FMath::Lerp(0.8f, 1.3f, SpeedNorm));
    }
}

void ARealWorldBoatBase::SetThrottleInput(float Value)
{
    ThrottleInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ARealWorldBoatBase::SetSteeringInput(float Value)
{
    SteeringInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

bool ARealWorldBoatBase::MountDriver(ARealWorldCharacter* Driver)
{
    if (!Driver || CurrentDriver) return false;

    CurrentDriver = Driver;
    Driver->SetActorHiddenInGame(true);
    Driver->SetActorEnableCollision(false);
    Driver->AttachToComponent(BoatMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("DriverSeatSocket"));

    if (APlayerController* PC = Cast<APlayerController>(Driver->GetController()))
    {
        PC->Possess(this);
    }

    return true;
}

void ARealWorldBoatBase::DismountDriver()
{
    if (!CurrentDriver) return;

    ARealWorldCharacter* Driver = CurrentDriver;
    CurrentDriver = nullptr;

    Driver->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Driver->SetActorHiddenInGame(false);
    Driver->SetActorEnableCollision(true);

    const FVector ExitLoc = GetActorLocation() + (-GetActorRightVector() * 180.0f) + FVector(0.0f, 0.0f, 50.0f);
    Driver->SetActorLocation(ExitLoc);

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->Possess(Driver);
    }

    OnBoatDismounted.Broadcast(Driver);
}
