// ============================================================================
// RealWorldVehicleDamageComponent.cpp
// Project RealWorld — GTA5-class Vehicle Damage
// ============================================================================

#include "Vehicles/RealWorldVehicleDamageComponent.h"
#include "Vehicles/RealWorldVehicleBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

URealWorldVehicleDamageComponent::URealWorldVehicleDamageComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void URealWorldVehicleDamageComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    OwnerVehicle  = Cast<ARealWorldVehicleBase>(GetOwner());

    // Initialize tyre states (4 tyres)
    DamageState.TyreStates.Init(ETyreState::Inflated, 4);
}

void URealWorldVehicleDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                      FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (DamageState.bIsOnFire && !bExploded)
    {
        TickFireDamage(DeltaTime);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// APPLY IMPACT DAMAGE
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldVehicleDamageComponent::ApplyImpactDamage(float Damage,
                                                          const FVector& ImpactPoint,
                                                          const FVector& ImpactNormal)
{
    if (bExploded) return;

    CurrentHealth = FMath::Max(CurrentHealth - Damage, 0.0f);
    DamageState.BodyIntegrity = CurrentHealth / MaxHealth;

    // Visual deformation on mesh
    DeformBodyAtPoint(ImpactPoint, Damage);

    // Spawn sparks
    SpawnImpactSparks(ImpactPoint);

    // Impact SFX
    if (ImpactMetalSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactMetalSound, ImpactPoint,
                                               FMath::Clamp(Damage / 200.0f, 0.1f, 1.0f));
    }

    // Engine state update
    UpdateEngineState();

    // Fire ignition check
    if (CurrentHealth <= FireIgnitionThreshold && !DamageState.bIsOnFire)
    {
        IgniteFire();
    }

    // Destruction
    if (CurrentHealth <= 0.0f)
    {
        Explode();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// DEFORMATION (Vertex displacement via Morph Targets or physics impulse)
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldVehicleDamageComponent::DeformBodyAtPoint(const FVector& WorldPoint, float DamageAmount)
{
    if (!OwnerVehicle) return;

    USkeletalMeshComponent* VehicleMesh = OwnerVehicle->GetMesh();
    if (!VehicleMesh) return;

    // Convert to local space
    const FVector LocalPoint = VehicleMesh->GetComponentTransform().InverseTransformPosition(WorldPoint);
    const EVehicleDamageZone Zone = ClassifyImpactZone(LocalPoint);

    // Calculate deformation morph target weight (normalized by max health)
    const float DeformWeight = FMath::Clamp((DamageAmount / MaxHealth) * DeformationScale * 5.0f, 0.0f, 1.0f);
    const float BodyDamage   = 1.0f - DamageState.BodyIntegrity;

    // Map zone to morph target name
    FName MorphTargetName = NAME_None;
    switch (Zone)
    {
        case EVehicleDamageZone::FrontLeft:  MorphTargetName = TEXT("Damage_FrontLeft");  break;
        case EVehicleDamageZone::FrontRight: MorphTargetName = TEXT("Damage_FrontRight"); break;
        case EVehicleDamageZone::RearLeft:   MorphTargetName = TEXT("Damage_RearLeft");   break;
        case EVehicleDamageZone::RearRight:  MorphTargetName = TEXT("Damage_RearRight");  break;
        case EVehicleDamageZone::Hood:       MorphTargetName = TEXT("Damage_Hood");       break;
        case EVehicleDamageZone::Trunk:      MorphTargetName = TEXT("Damage_Trunk");      break;
        case EVehicleDamageZone::Roof:       MorphTargetName = TEXT("Damage_Roof");       break;
    }

    if (MorphTargetName != NAME_None)
    {
        // Accumulate morph target weight (clamp to 1)
        const float CurrentWeight = VehicleMesh->GetMorphTarget(MorphTargetName);
        VehicleMesh->SetMorphTarget(MorphTargetName,
                                     FMath::Min(CurrentWeight + DeformWeight, 1.0f));
    }

    // Scale mesh material dirt/scratch parameter
    UMaterialInstanceDynamic* MatInst = VehicleMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (MatInst)
    {
        MatInst->SetScalarParameterValue(TEXT("DamageAmount"), BodyDamage);
    }
}

EVehicleDamageZone URealWorldVehicleDamageComponent::ClassifyImpactZone(const FVector& LocalPoint) const
{
    // Simple zone classification based on local position
    const bool bFront  = LocalPoint.X > 0.0f;
    const bool bLeft   = LocalPoint.Y < 0.0f;
    const bool bTop    = LocalPoint.Z > 50.0f;

    if (bTop)    return EVehicleDamageZone::Roof;
    if (bFront)  return bLeft ? EVehicleDamageZone::FrontLeft  : EVehicleDamageZone::FrontRight;
    return              bLeft ? EVehicleDamageZone::RearLeft   : EVehicleDamageZone::RearRight;
}

// ─────────────────────────────────────────────────────────────────────────────
// ENGINE STATE
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldVehicleDamageComponent::UpdateEngineState()
{
    const float HealthPct = GetHealthPercent();
    EEngineState NewState;

    if (HealthPct > 0.5f)       NewState = EEngineState::Normal;
    else if (HealthPct > 0.2f)  NewState = EEngineState::Damaged;
    else if (HealthPct > 0.0f)  NewState = EEngineState::Critical;
    else                        NewState = EEngineState::Destroyed;

    if (NewState == DamageState.EngineState) return;

    DamageState.EngineState = NewState;
    OnEngineStateChanged.Broadcast(NewState);

    if (!OwnerVehicle) return;

    // Spawn smoke/fire VFX on engine
    const FVector EngineLocation = OwnerVehicle->GetMesh()
        ->GetSocketLocation(TEXT("EngineExhaust"));

    switch (NewState)
    {
        case EEngineState::Damaged:
            if (EngineSmokeLight_FX)
            {
                ActiveSmokeFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    GetWorld(), EngineSmokeLight_FX, EngineLocation,
                    FRotator::ZeroRotator, FVector::OneVector,
                    true, true, ENCPoolMethod::None
                );
            }
            break;

        case EEngineState::Critical:
            if (ActiveSmokeFX) { ActiveSmokeFX->DestroyComponent(); ActiveSmokeFX = nullptr; }
            if (EngineFireFX)
            {
                ActiveFireFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    GetWorld(), EngineFireFX, EngineLocation,
                    FRotator::ZeroRotator, FVector::OneVector,
                    true, true, ENCPoolMethod::None
                );
            }
            break;

        case EEngineState::Destroyed:
            break;

        default: break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TYRE DAMAGE
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldVehicleDamageComponent::PunctureRandomTyre()
{
    // Find a still-inflated tyre
    TArray<int32> InflatedTyres;
    for (int32 i = 0; i < DamageState.TyreStates.Num(); ++i)
    {
        if (DamageState.TyreStates[i] == ETyreState::Inflated)
            InflatedTyres.Add(i);
    }

    if (InflatedTyres.Num() == 0) return;

    const int32 RandomIndex = InflatedTyres[FMath::RandRange(0, InflatedTyres.Num() - 1)];
    PunctureTyre(RandomIndex);
}

void URealWorldVehicleDamageComponent::PunctureTyre(int32 TyreIndex)
{
    if (!DamageState.TyreStates.IsValidIndex(TyreIndex)) return;
    if (DamageState.TyreStates[TyreIndex] == ETyreState::Deflated) return;

    DamageState.TyreStates[TyreIndex] = ETyreState::Deflated;

    if (TyreBurstSound && OwnerVehicle)
    {
        UGameplayStatics::PlaySoundAtLocation(this, TyreBurstSound, OwnerVehicle->GetActorLocation());
    }

    OnTyreDamaged.Broadcast(TyreIndex, ETyreState::Deflated);
}

// ─────────────────────────────────────────────────────────────────────────────
// FIRE
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldVehicleDamageComponent::IgniteFire()
{
    if (DamageState.bIsOnFire || bExploded) return;
    DamageState.bIsOnFire = true;
    DamageState.FireTimer  = 0.0f;

    if (FireLoopSound && OwnerVehicle)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireLoopSound, OwnerVehicle->GetActorLocation());
    }
}

void URealWorldVehicleDamageComponent::TickFireDamage(float DeltaTime)
{
    DamageState.FireTimer += DeltaTime;

    // Fire deals continuous damage
    ApplyImpactDamage(FireDamagePerSecond * DeltaTime,
                      OwnerVehicle ? OwnerVehicle->GetActorLocation() : FVector::ZeroVector,
                      FVector::UpVector);
}

// ─────────────────────────────────────────────────────────────────────────────
// EXPLOSION
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldVehicleDamageComponent::Explode()
{
    if (bExploded) return;
    bExploded = true;
    CurrentHealth = 0.0f;
    DamageState.bIsOnFire = false;

    if (!OwnerVehicle) return;

    const FVector ExplosionOrigin = OwnerVehicle->GetActorLocation();

    // Spawn explosion VFX
    if (ExplosionFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), ExplosionFX, ExplosionOrigin,
            FRotator::ZeroRotator, FVector(3.0f), // Big scale
            true, true, ENCPoolMethod::None
        );
    }

    // Explosion SFX
    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, ExplosionOrigin);
    }

    // Radial damage to nearby actors
    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        ExplosionDamage,
        ExplosionOrigin,
        ExplosionRadius,
        UDamageType::StaticClass(),
        TArray<AActor*>({ OwnerVehicle }),
        OwnerVehicle,
        nullptr,
        true // Full damage
    );

    // Physics impulse to launch vehicle
    if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(OwnerVehicle->GetRootComponent()))
    {
        Root->AddImpulse(FVector(0.0f, 0.0f, 300000.0f));
        Root->AddAngularImpulseInDegrees(FVector(
            FMath::RandRange(-50.0f, 50.0f),
            FMath::RandRange(-50.0f, 50.0f),
            FMath::RandRange(-180.0f, 180.0f)
        ));
    }

    OnVehicleExploded.Broadcast();

    // Destroy vehicle after brief delay (let VFX play)
    OwnerVehicle->SetActorHiddenInGame(false);
    OwnerVehicle->SetLifeSpan(8.0f); // Remove from world in 8s
}

// ─────────────────────────────────────────────────────────────────────────────
// REPAIR
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldVehicleDamageComponent::Repair(float Amount)
{
    if (bExploded) return;

    CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);
    DamageState.BodyIntegrity = CurrentHealth / MaxHealth;
    DamageState.bIsOnFire = false;

    UpdateEngineState();

    // Reset morph targets
    if (OwnerVehicle && OwnerVehicle->GetMesh())
    {
        static const TArray<FName> DamageMorphs = {
            TEXT("Damage_FrontLeft"), TEXT("Damage_FrontRight"),
            TEXT("Damage_RearLeft"),  TEXT("Damage_RearRight"),
            TEXT("Damage_Hood"),      TEXT("Damage_Trunk"), TEXT("Damage_Roof")
        };
        for (const FName& Morph : DamageMorphs)
        {
            OwnerVehicle->GetMesh()->SetMorphTarget(Morph, 0.0f);
        }
    }
}

void URealWorldVehicleDamageComponent::RepairVehicle()
{
    Repair(MaxHealth);
    for (int32 i = 0; i < DamageState.TyreStates.Num(); ++i)
    {
        DamageState.TyreStates[i] = ETyreState::Inflated;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SPARKS
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldVehicleDamageComponent::SpawnImpactSparks(const FVector& ImpactPoint) const
{
    if (SparksOnImpactFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), SparksOnImpactFX, ImpactPoint,
            FRotator::ZeroRotator, FVector::OneVector,
            true, true, ENCPoolMethod::AutoRelease
        );
    }
}
