// ============================================================================
// RealWorldProjectileRocket.cpp
// Project RealWorld — GTA5-class RPG Rocket Launcher Projectile
// ============================================================================

#include "Combat/RealWorldProjectileRocket.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"

ARealWorldProjectileRocket::ARealWorldProjectileRocket()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->InitSphereRadius(12.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
    RocketMesh->SetupAttachment(RootComponent);
    RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionSphere;
    ProjectileMovement->InitialSpeed = 3800.0f; // Fast rocket flight
    ProjectileMovement->MaxSpeed = 4500.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.15f; // Slight drop over long range

    BaseExplosionDamage = 350.0f;
    InnerExplosionRadius = 250.0f;
    OuterExplosionRadius = 800.0f;
    ExplosionImpulseForce = 150000.0f;

    InitialLifeSpan = 10.0f;
}

void ARealWorldProjectileRocket::BeginPlay()
{
    Super::BeginPlay();

    CollisionSphere->OnComponentHit.AddDynamic(this, &ARealWorldProjectileRocket::OnRocketHit);
}

void ARealWorldProjectileRocket::OnRocketHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                                             const FHitResult& Hit)
{
    const FVector ExplodeLoc = Hit.ImpactPoint.IsZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);
    Detonate(ExplodeLoc);
}

void ARealWorldProjectileRocket::Detonate(const FVector& ExplosionLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 1. Radial damage with falloff
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this);

    UGameplayStatics::ApplyRadialDamageWithFalloff(
        World,
        BaseExplosionDamage,
        35.0f, // minimum damage at edge
        ExplosionLocation,
        InnerExplosionRadius,
        OuterExplosionRadius,
        1.0f,
        UDamageType::StaticClass(),
        IgnoredActors,
        this,
        GetInstigatorController()
    );

    // 2. Physical radial impulse to flip vehicles and throw ragdolls
    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(OuterExplosionRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (World->OverlapMultiByChannel(Overlaps, ExplosionLocation, FQuat::Identity, ECC_PhysicsBody, SphereShape, Params))
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            if (UPrimitiveComponent* Prim = Overlap.GetComponent())
            {
                if (Prim->IsSimulatingPhysics())
                {
                    Prim->AddRadialImpulse(ExplosionLocation, OuterExplosionRadius, ExplosionImpulseForce, ERadialImpulseFalloff::RIF_Linear, true);
                }
            }
        }
    }

    // 3. VFX / SFX
    if (ExplosionVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, ExplosionVFX, ExplosionLocation);
    }

    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(World, ExplosionSound, ExplosionLocation);
    }

    Destroy();
}
