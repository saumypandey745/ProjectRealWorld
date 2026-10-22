// ============================================================================
// RealWorldSuperyachtActor.cpp
// Project RealWorld — GTA Online Galaxy Superyacht Simulation
// ============================================================================

#include "World/RealWorldSuperyachtActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Vehicles/RealWorldHelicopterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

ARealWorldSuperyachtActor::ARealWorldSuperyachtActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Yacht Hull
    YachtHullMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("YachtHullMesh"));
    YachtHullMesh->SetCollisionProfileName(TEXT("BlockAll"));
    SetRootComponent(YachtHullMesh);

    // Helipad on upper deck
    HelipadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HelipadMesh"));
    HelipadMesh->SetupAttachment(YachtHullMesh);
    HelipadMesh->SetRelativeLocation(FVector(-3500.0f, 0.0f, 850.0f));
    HelipadMesh->SetCollisionProfileName(TEXT("BlockAll"));

    // Helipad flashing beacon
    HelipadBeaconLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("HelipadBeaconLight"));
    HelipadBeaconLight->SetupAttachment(HelipadMesh);
    HelipadBeaconLight->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
    HelipadBeaconLight->SetLightColor(FLinearColor(0.1f, 1.0f, 0.2f));
    HelipadBeaconLight->SetIntensity(10000.0f);

    // Air defense perimeter zone
    DefenseZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DefenseZoneSphere"));
    DefenseZoneSphere->SetupAttachment(YachtHullMesh);
    DefenseZoneSphere->SetSphereRadius(AirDefenseRadius);
    DefenseZoneSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ARealWorldSuperyachtActor::BeginPlay()
{
    Super::BeginPlay();
}

void ARealWorldSuperyachtActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 1. Helipad beacon pulsing light (GTA green flash)
    HelipadPulseTimer += DeltaTime * 3.0f;
    if (HelipadBeaconLight)
    {
        const float Brightness = (FMath::Sin(HelipadPulseTimer) > 0.0f) ? 12000.0f : 1000.0f;
        HelipadBeaconLight->SetIntensity(Brightness);
    }

    // 2. Air defense tracking
    if (bAirDefensesActive)
    {
        ScanAirDefensePerimeter(DeltaTime);
    }
}

void ARealWorldSuperyachtActor::ScanAirDefensePerimeter(float DeltaTime)
{
    DefenseCooldownTimer -= DeltaTime;

    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(AirDefenseRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere, Params);

    for (const FOverlapResult& Res : Overlaps)
    {
        if (AActor* Candidate = Res.GetActor())
        {
            if (Candidate->IsA<ARealWorldHelicopterBase>())
            {
                if (DefenseCooldownTimer <= 0.0f)
                {
                    FireDefenseMissile(Candidate);
                    DefenseCooldownTimer = DefenseFireInterval;
                    break;
                }
            }
        }
    }
}

void ARealWorldSuperyachtActor::FireDefenseMissile(AActor* HostileAircraft)
{
    if (!HostileAircraft) return;

    if (AirDefenseWarningSiren)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AirDefenseWarningSiren, GetActorLocation());
    }

    if (MissileLaunchSound)
    {
        const FVector TurretLoc = GetActorLocation() + FVector(0.0f, 0.0f, 1200.0f);
        UGameplayStatics::PlaySoundAtLocation(this, MissileLaunchSound, TurretLoc);
    }
}

void ARealWorldSuperyachtActor::SetAirDefenses(bool bEnabled)
{
    bAirDefensesActive = bEnabled;
    OnSuperyachtDefensesToggled.Broadcast(bAirDefensesActive);
}
