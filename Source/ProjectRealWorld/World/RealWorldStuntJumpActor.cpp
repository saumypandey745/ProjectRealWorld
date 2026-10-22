// ============================================================================
// RealWorldStuntJumpActor.cpp
// Project RealWorld — GTA V-Class Unique Stunt Jumps System
// ============================================================================

#include "World/RealWorldStuntJumpActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Vehicles/RealWorldVehicleBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ARealWorldStuntJumpActor::ARealWorldStuntJumpActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Physical Ramp mesh
    RampMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RampMesh"));
    RampMesh->SetCollisionProfileName(TEXT("BlockAll"));
    SetRootComponent(RampMesh);

    // Launch box at edge of ramp
    LaunchTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("LaunchTrigger"));
    LaunchTrigger->SetupAttachment(RampMesh);
    LaunchTrigger->SetBoxExtent(FVector(200.0f, 400.0f, 150.0f));
    LaunchTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    // Expected landing area
    LandingZone = CreateDefaultSubobject<UBoxComponent>(TEXT("LandingZone"));
    LandingZone->SetupAttachment(RampMesh);
    LandingZone->SetRelativeLocation(FVector(4000.0f, 0.0f, -300.0f));
    LandingZone->SetBoxExtent(FVector(1000.0f, 1000.0f, 300.0f));
    LandingZone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ARealWorldStuntJumpActor::BeginPlay()
{
    Super::BeginPlay();
    LaunchTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARealWorldStuntJumpActor::OnLaunchOverlap);
}

void ARealWorldStuntJumpActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bJumpActive && ActiveJumpingVehicle)
    {
        AirborneTimer += DeltaTime;

        // Check if vehicle has landed or timeout (7 seconds max)
        const FVector Vel = ActiveJumpingVehicle->GetVelocity();
        const bool bNearGround = ActiveJumpingVehicle->GetActorLocation().Z < (GetActorLocation().Z - 200.0f);

        if (AirborneTimer > 1.2f && (Vel.Size() < 200.0f || bNearGround || AirborneTimer > 6.0f))
        {
            // Evaluate landing: upright rotation (Pitch and Roll within +/- 35 degrees)
            const FRotator Rot = ActiveJumpingVehicle->GetActorRotation();
            const bool bUpright = (FMath::Abs(Rot.Pitch) < 35.0f && FMath::Abs(Rot.Roll) < 35.0f);
            EndStuntJump(bUpright);
        }
    }
}

void ARealWorldStuntJumpActor::OnLaunchOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                              bool bFromSweep, const FHitResult& SweepResult)
{
    if (bJumpActive || !OtherActor) return;

    ARealWorldVehicleBase* Vehicle = Cast<ARealWorldVehicleBase>(OtherActor);
    if (!Vehicle) return;

    const float SpeedKmh = Vehicle->GetVelocity().Size() * 0.036f;
    if (SpeedKmh < MinimumLaunchSpeedKmh) return;

    bJumpActive = true;
    AirborneTimer = 0.0f;
    ActiveJumpingVehicle = Vehicle;

    // Slow motion cinematic jump camera
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGlobalTimeDilation(World, 0.35f);
    }
}

void ARealWorldStuntJumpActor::EndStuntJump(bool bSuccessful)
{
    if (!bJumpActive) return;
    bJumpActive = false;

    // Restore normal game speed
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
    }

    if (bSuccessful)
    {
        if (JumpSuccessSound)
        {
            UGameplayStatics::PlaySound2D(this, JumpSuccessSound);
        }
        OnStuntJumpFinished.Broadcast(true, CashReward);
    }
    else
    {
        if (JumpFailSound)
        {
            UGameplayStatics::PlaySound2D(this, JumpFailSound);
        }
        OnStuntJumpFinished.Broadcast(false, 0);
    }

    ActiveJumpingVehicle = nullptr;
}
