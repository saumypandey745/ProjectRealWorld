// ============================================================================
// RealWorldHelicopterBase.cpp
// Project RealWorld — GTA V-Class Attack Helicopter Simulation
// ============================================================================

#include "Vehicles/RealWorldHelicopterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "Character/RealWorldCharacter.h"
#include "Combat/RealWorldProjectileRocket.h"

ARealWorldHelicopterBase::ARealWorldHelicopterBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root skeletal mesh (Fuselage)
    HelicopterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelicopterMesh"));
    HelicopterMesh->SetSimulatePhysics(false);
    HelicopterMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    SetRootComponent(HelicopterMesh);

    // Main Rotor
    MainRotorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainRotorMesh"));
    MainRotorMesh->SetupAttachment(HelicopterMesh, TEXT("MainRotorSocket"));
    MainRotorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Tail Rotor
    TailRotorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TailRotorMesh"));
    TailRotorMesh->SetupAttachment(HelicopterMesh, TEXT("TailRotorSocket"));
    TailRotorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Night Searchlight
    Searchlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Searchlight"));
    Searchlight->SetupAttachment(HelicopterMesh, TEXT("NoseCameraSocket"));
    Searchlight->SetIntensity(50000.0f);
    Searchlight->SetOuterConeAngle(30.0f);
    Searchlight->SetInnerConeAngle(15.0f);
    Searchlight->SetLightColor(FLinearColor(1.0f, 0.98f, 0.92f));
    Searchlight->SetVisibility(false);
}

void ARealWorldHelicopterBase::BeginPlay()
{
    Super::BeginPlay();

    if (EngineRotorSound)
    {
        RotorAudioComp = UGameplayStatics::SpawnSoundAttached(
            EngineRotorSound, HelicopterMesh, NAME_None,
            FVector::ZeroVector, EAttachLocation::KeepRelativeOffset,
            true
        );
        if (RotorAudioComp)
        {
            RotorAudioComp->SetVolumeMultiplier(0.0f);
        }
    }
}

void ARealWorldHelicopterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ARealWorldHelicopterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateRotorAnimation(DeltaTime);
    UpdateFlightPhysics(DeltaTime);
    UpdateWeapons(DeltaTime);
}

void ARealWorldHelicopterBase::UpdateRotorAnimation(float DeltaTime)
{
    const float TargetRPM = bEngineStarted ? 1000.0f : 0.0f;
    CurrentRotorRPM = FMath::FInterpTo(CurrentRotorRPM, TargetRPM, DeltaTime, 1.0f / RotorSpoolUpTime);

    if (MainRotorMesh)
    {
        const FRotator MainDelta = FRotator(0.0f, CurrentRotorRPM * 360.0f / 60.0f * DeltaTime, 0.0f);
        MainRotorMesh->AddLocalRotation(MainDelta);
    }

    if (TailRotorMesh)
    {
        const FRotator TailDelta = FRotator(CurrentRotorRPM * 4.0f * 360.0f / 60.0f * DeltaTime, 0.0f, 0.0f);
        TailRotorMesh->AddLocalRotation(TailDelta);
    }

    if (RotorAudioComp)
    {
        const float Volume = FMath::Clamp(CurrentRotorRPM / 1000.0f, 0.0f, 1.0f);
        RotorAudioComp->SetVolumeMultiplier(Volume);
        RotorAudioComp->SetPitchMultiplier(FMath::Lerp(0.5f, 1.0f, Volume));
    }
}

void ARealWorldHelicopterBase::UpdateFlightPhysics(float DeltaTime)
{
    if (CurrentRotorRPM < 400.0f) return;

    const float LiftEfficiency = FMath::Clamp(CurrentRotorRPM / 1000.0f, 0.0f, 1.0f);

    // 1. Altitude (Collective)
    const float TargetZVelocity = CollectiveInput * MaxCollectiveLift * LiftEfficiency;
    CurrentVelocity.Z = FMath::FInterpTo(CurrentVelocity.Z, TargetZVelocity, DeltaTime, 3.0f);

    // 2. Pitch (Forward/Back)
    TargetPitch = FMath::FInterpTo(TargetPitch, PitchInput * MaxPitchAngle, DeltaTime, 4.0f);

    // 3. Roll (Banking Left/Right)
    TargetRoll = FMath::FInterpTo(TargetRoll, RollInput * MaxRollAngle, DeltaTime, 4.0f);

    // 4. Horizontal velocity from tilt
    const FVector ForwardThrust = GetActorForwardVector() * (-TargetPitch / MaxPitchAngle) * MaxHorizontalSpeed;
    const FVector RightThrust   = GetActorRightVector()   * (TargetRoll / MaxRollAngle)   * (MaxHorizontalSpeed * 0.75f);
    
    CurrentVelocity.X = FMath::FInterpTo(CurrentVelocity.X, (ForwardThrust + RightThrust).X, DeltaTime, 2.5f);
    CurrentVelocity.Y = FMath::FInterpTo(CurrentVelocity.Y, (ForwardThrust + RightThrust).Y, DeltaTime, 2.5f);

    // Apply movement
    const FVector DeltaLocation = CurrentVelocity * DeltaTime;
    FHitResult MoveHit;
    AddActorWorldOffset(DeltaLocation, true, &MoveHit);

    // Yaw rotation
    const float DeltaYaw = YawInput * YawTurnRate * DeltaTime;
    FRotator CurrentRot = GetActorRotation();
    CurrentRot.Yaw += DeltaYaw;
    CurrentRot.Pitch = TargetPitch;
    CurrentRot.Roll = TargetRoll;
    SetActorRotation(CurrentRot);
}

// ─── Controls ────────────────────────────────────────────────────────────────
void ARealWorldHelicopterBase::SetCollectiveInput(float Value)
{
    CollectiveInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ARealWorldHelicopterBase::SetPitchInput(float Value)
{
    PitchInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ARealWorldHelicopterBase::SetRollInput(float Value)
{
    RollInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ARealWorldHelicopterBase::SetYawInput(float Value)
{
    YawInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

// ─── Weapons ─────────────────────────────────────────────────────────────────
void ARealWorldHelicopterBase::StartFiringMiniguns()
{
    bFiringMiniguns = true;
}

void ARealWorldHelicopterBase::StopFiringMiniguns()
{
    bFiringMiniguns = false;
}

void ARealWorldHelicopterBase::UpdateWeapons(float DeltaTime)
{
    if (MinigunCooldownTimer > 0.0f)
    {
        MinigunCooldownTimer -= DeltaTime;
    }

    if (bFiringMiniguns && MinigunCooldownTimer <= 0.0f)
    {
        FireMinigunBullet();
        MinigunCooldownTimer = MinigunFireRate;
    }

    if (MissileCooldownTimer > 0.0f)
    {
        MissileCooldownTimer -= DeltaTime;
    }

    SearchTargetLock();
}

void ARealWorldHelicopterBase::FireMinigunBullet()
{
    const FName SocketName = bNextGunLeft ? TEXT("LeftMinigunSocket") : TEXT("RightMinigunSocket");
    bNextGunLeft = !bNextGunLeft;

    const FVector MuzzleLoc = HelicopterMesh->GetSocketLocation(SocketName);
    const FVector TraceEnd = MuzzleLoc + (GetActorForwardVector() * 10000.0f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (CurrentPilot) Params.AddIgnoredActor(CurrentPilot);

    GetWorld()->LineTraceSingleByChannel(Hit, MuzzleLoc, TraceEnd, ECC_Visibility, Params);

    if (Hit.bBlockingHit && Hit.GetActor())
    {
        FPointDamageEvent DmgEvent(MinigunDamage, Hit, GetActorForwardVector(), nullptr);
        Hit.GetActor()->TakeDamage(MinigunDamage, DmgEvent, GetController(), this);
    }

    if (MinigunFireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, MinigunFireSound, MuzzleLoc);
    }
}

void ARealWorldHelicopterBase::SearchTargetLock()
{
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(MissileLockRange);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (CurrentPilot) Params.AddIgnoredActor(CurrentPilot);

    GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere, Params);

    AActor* BestTarget = nullptr;
    float BestDot = 0.85f; // Must be in front cone (approx 30 degrees)

    for (const FOverlapResult& Res : Overlaps)
    {
        if (AActor* Candidate = Res.GetActor())
        {
            const FVector DirToCandidate = (Candidate->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            const float Dot = FVector::DotProduct(GetActorForwardVector(), DirToCandidate);
            if (Dot > BestDot)
            {
                BestDot = Dot;
                BestTarget = Candidate;
            }
        }
    }

    if (BestTarget != CurrentLockedTarget)
    {
        CurrentLockedTarget = BestTarget;
        if (CurrentLockedTarget && TargetLockSound)
        {
            UGameplayStatics::PlaySound2D(this, TargetLockSound);
        }
        OnHelicopterTargetLocked.Broadcast(CurrentLockedTarget);
    }
}

void ARealWorldHelicopterBase::FireMissileVolley()
{
    if (MissileCooldownTimer > 0.0f || !RocketClass) return;

    MissileCooldownTimer = MissileCooldown;

    const FVector SpawnLoc = HelicopterMesh->GetSocketLocation(TEXT("MissilePodSocket"));
    FRotator SpawnRot = GetActorRotation();

    if (CurrentLockedTarget)
    {
        SpawnRot = (CurrentLockedTarget->GetActorLocation() - SpawnLoc).Rotation();
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;

    GetWorld()->SpawnActor<ARealWorldProjectileRocket>(RocketClass, SpawnLoc, SpawnRot, SpawnParams);

    if (MissileLaunchSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, MissileLaunchSound, SpawnLoc);
    }
}

// ─── Pilot Mounting ──────────────────────────────────────────────────────────
bool ARealWorldHelicopterBase::MountPilot(ARealWorldCharacter* Pilot)
{
    if (!Pilot || CurrentPilot) return false;

    CurrentPilot = Pilot;
    bEngineStarted = true;

    Pilot->SetActorHiddenInGame(true);
    Pilot->SetActorEnableCollision(false);
    Pilot->AttachToComponent(HelicopterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("PilotSeatSocket"));

    if (APlayerController* PC = Cast<APlayerController>(Pilot->GetController()))
    {
        PC->Possess(this);
    }

    if (Searchlight)
    {
        Searchlight->SetVisibility(true);
    }

    OnHelicopterStateChanged.Broadcast(true);
    return true;
}

void ARealWorldHelicopterBase::DismountPilot()
{
    if (!CurrentPilot) return;

    ARealWorldCharacter* Pilot = CurrentPilot;
    CurrentPilot = nullptr;
    bEngineStarted = false;

    Pilot->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Pilot->SetActorHiddenInGame(false);
    Pilot->SetActorEnableCollision(true);

    const FVector ExitLoc = GetActorLocation() + (-GetActorRightVector() * 250.0f);
    Pilot->SetActorLocation(ExitLoc);

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->Possess(Pilot);
    }

    if (Searchlight)
    {
        Searchlight->SetVisibility(false);
    }

    OnHelicopterStateChanged.Broadcast(false);
}
