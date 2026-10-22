// ============================================================================
// RealWorldMotorcycleBase.cpp
// Project RealWorld — GTA5-class 2-Wheel Motorcycle Physics & Leaning
// ============================================================================

#include "Vehicles/RealWorldMotorcycleBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Character/RealWorldCharacter.h"
#include "Character/RealWorldHealthComponent.h"
#include "Kismet/GameplayStatics.h"

ARealWorldMotorcycleBase::ARealWorldMotorcycleBase()
{
    PrimaryActorTick.bCanEverTick = true;

    BikeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BikeMesh"));
    RootComponent = BikeMesh;
    BikeMesh->SetSimulatePhysics(true);
    BikeMesh->SetCollisionProfileName(TEXT("Vehicle"));

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 350.0f;
    SpringArm->SocketOffset = FVector(0.0f, 0.0f, 90.0f);
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 12.0f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    MaxSpeed = 2400.0f;
    AccelerationRate = 1200.0f;
    BrakeStrength = 1800.0f;
    TurnRate = 65.0f;
    MaxLeanAngle = 42.0f;
    LeanInterpSpeed = 8.0f;
    WheeliePitchAngle = 30.0f;
    EjectionSpeedThreshold = 750.0f;
}

void ARealWorldMotorcycleBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ApplyDrivePhysics(DeltaTime);
    UpdateLeanAndWheelie(DeltaTime);
}

void ARealWorldMotorcycleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ARealWorldMotorcycleBase::ApplyDrivePhysics(float DeltaTime)
{
    if (!BikeMesh) return;

    CurrentSpeed = BikeMesh->GetPhysicsLinearVelocity().Size();

    // Forward drive impulse
    if (FMath::Abs(ThrottleInput) > 0.01f)
    {
        const FVector ForwardForce = GetActorForwardVector() * ThrottleInput * AccelerationRate * 100.0f;
        BikeMesh->AddForce(ForwardForce);
    }

    // Steering yaw torque
    if (FMath::Abs(SteeringInput) > 0.01f && CurrentSpeed > 100.0f)
    {
        const float SpeedFactor = FMath::Clamp(CurrentSpeed / MaxSpeed, 0.2f, 1.0f);
        const FVector YawTorque = FVector(0.0f, 0.0f, SteeringInput * TurnRate * SpeedFactor * 50000.0f);
        BikeMesh->AddTorqueInDegrees(YawTorque);
    }
}

void ARealWorldMotorcycleBase::UpdateLeanAndWheelie(float DeltaTime)
{
    // Roll lean banking: when steering right at speed, lean right (roll positive)
    const float TargetLean = SteeringInput * MaxLeanAngle * FMath::Clamp(CurrentSpeed / 1200.0f, 0.0f, 1.0f);
    CurrentLeanRoll = FMath::FInterpTo(CurrentLeanRoll, TargetLean, DeltaTime, LeanInterpSpeed);

    // Wheelie pitch: pull back joystick (PitchInput < 0) + throttle
    const float TargetPitch = (PitchInput < -0.3f && ThrottleInput > 0.5f) ? WheeliePitchAngle : 0.0f;
    CurrentWheeliePitch = FMath::FInterpTo(CurrentWheeliePitch, TargetPitch, DeltaTime, 6.0f);
    bIsWheelie = (CurrentWheeliePitch > 10.0f);

    // Apply lean visual offset to root mesh
    FRotator CurrentRot = GetActorRotation();
    CurrentRot.Roll = CurrentLeanRoll;
    CurrentRot.Pitch += CurrentWheeliePitch;
}

void ARealWorldMotorcycleBase::MountRider(ARealWorldCharacter* Rider)
{
    if (!Rider) return;

    MountedRider = Rider;
    Rider->AttachToComponent(BikeMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("RiderSocket"));
    Rider->SetActorEnableCollision(false);

    if (APlayerController* PC = Cast<APlayerController>(Rider->GetController()))
    {
        PC->Possess(this);
    }
}

void ARealWorldMotorcycleBase::DismountRider()
{
    if (!MountedRider) return;

    MountedRider->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    MountedRider->SetActorEnableCollision(true);

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->Possess(MountedRider);
    }

    MountedRider = nullptr;
}

void ARealWorldMotorcycleBase::EjectRider(const FVector& ImpactVelocity)
{
    if (!MountedRider) return;

    ARealWorldCharacter* EjectedChar = MountedRider;
    DismountRider();

    // Activate GTA ragdoll with forward launch impulse
    if (URealWorldHealthComponent* Health = EjectedChar->FindComponentByClass<URealWorldHealthComponent>())
    {
        Health->ApplyRagdollImpulse(ImpactVelocity * 2.0f);
    }

    if (CrashSound && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(this, CrashSound, GetActorLocation());
    }

    OnRiderEjected.Broadcast();
}

void ARealWorldMotorcycleBase::OnBikeHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                                         const FHitResult& Hit)
{
    const float ImpactSpeed = NormalImpulse.Size();
    if (ImpactSpeed >= EjectionSpeedThreshold && MountedRider != nullptr)
    {
        EjectRider(BikeMesh ? BikeMesh->GetPhysicsLinearVelocity() : FVector::ForwardVector * 1000.0f);
    }
}
