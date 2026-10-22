// ============================================================================
// RealWorldWeaponBase.cpp
// Project RealWorld — GTA5-class Weapon System
// Full ballistics, recoil, damage falloff, muzzle flash VFX, MetaSound SFX
// ============================================================================

#include "Combat/RealWorldWeaponBase.h"
#include "Character/RealWorldCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "Camera/CameraShakeBase.h"

// Surface type physical material channels (configured in DefaultEngine.ini)
namespace RWSurface
{
    static const EPhysicalSurface Flesh   = SurfaceType1;
    static const EPhysicalSurface Concrete = SurfaceType2;
    static const EPhysicalSurface Metal   = SurfaceType3;
    static const EPhysicalSurface Glass   = SurfaceType4;
    static const EPhysicalSurface Wood    = SurfaceType5;
    static const EPhysicalSurface Water   = SurfaceType6;
}

ARealWorldWeaponBase::ARealWorldWeaponBase()
{
    PrimaryActorTick.bCanEverTick = true;

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

    // Disable shadow on first-person mesh to avoid self-shadow artifacts
    WeaponMesh->bCastDynamicShadow = false;
    WeaponMesh->CastShadow = false;
}

void ARealWorldWeaponBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentAmmoInMag = MagazineCapacity;
}

// ─────────────────────────────────────────────────────────────────────────────
// TICK — Recoil Recovery
// ─────────────────────────────────────────────────────────────────────────────
void ARealWorldWeaponBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smoothly recover accumulated recoil offset each frame
    if (!CurrentRecoilOffset.IsNearlyZero(0.001f))
    {
        const float RecoveryAlpha = FMath::Clamp(RecoilRecoverySpeed * DeltaTime, 0.0f, 1.0f);
        CurrentRecoilOffset = FMath::Lerp(CurrentRecoilOffset, FVector2D::ZeroVector, RecoveryAlpha);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// FIRE
// ─────────────────────────────────────────────────────────────────────────────
bool ARealWorldWeaponBase::FireWeapon(ARealWorldCharacter* InstigatorPawn)
{
    if (!InstigatorPawn) return false;

    // Ammo check
    if (CurrentAmmoInMag <= 0)
    {
        PlayDryFireSound();
        return false;
    }

    // Fire-rate gate (RPM → seconds between shots)
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float FireInterval = 60.0f / FMath::Max(FireRateRPM, 1.0f);
    if ((CurrentTime - LastFireTime) < FireInterval) return false;

    LastFireTime = CurrentTime;
    CurrentAmmoInMag--;

    // --- Ballistic Raycast ---
    FHitResult HitResult;
    if (PerformBallisticTrace(InstigatorPawn, HitResult))
    {
        ProcessHit(InstigatorPawn, HitResult);
    }

    // --- VFX & SFX ---
    SpawnMuzzleFlash();
    SpawnCasingEject();
    PlayFireSound();
    ApplyRecoilToController(InstigatorPawn);
    ApplyCameraShake(InstigatorPawn);

    // Play upper body fire recoil montage
    if (InstigatorPawn && CharacterFireMontage)
    {
        InstigatorPawn->PlayAnimMontage(CharacterFireMontage);
    }

    OnWeaponFired.Broadcast(CurrentAmmoInMag, TotalReserveAmmo);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// BALLISTIC TRACE — Multi-sphere trace with penetration
// ─────────────────────────────────────────────────────────────────────────────
bool ARealWorldWeaponBase::PerformBallisticTrace(ARealWorldCharacter* Instigator, FHitResult& OutHit) const
{
    if (!Instigator) return false;

    AController* InstigatorController = Instigator->GetController();
    if (!InstigatorController) return false;

    FVector CameraLocation;
    FRotator CameraRotation;
    InstigatorController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    // Apply spread (increases when moving, decreases when aiming)
    const bool bAiming = (Instigator->CurrentUpperBodyState == EUpperBodyState::Aiming);
    const float SpreadMultiplier = bAiming ? 0.25f : (Instigator->GetVelocity().SizeSquared() > 100.0f ? 2.5f : 1.0f);
    const float AppliedSpread   = BaseSpreadDegrees * SpreadMultiplier;

    FVector FireDirection = CameraRotation.Vector();
    if (AppliedSpread > 0.0f)
    {
        const float HalfSpreadRad = FMath::DegreesToRadians(AppliedSpread * 0.5f);
        FireDirection = FMath::VRandCone(FireDirection, HalfSpreadRad);
    }

    const FVector TraceEnd = CameraLocation + FireDirection * EffectiveRange;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(Instigator);
    QueryParams.bTraceComplex         = true;
    QueryParams.bReturnPhysicalMaterial = true;

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHit, CameraLocation, TraceEnd,
        ECollisionChannel::ECC_Visibility, QueryParams
    );

#if WITH_EDITORONLY_DATA
    if (bDrawDebugTrace)
    {
        DrawDebugLine(GetWorld(), CameraLocation, bHit ? OutHit.ImpactPoint : TraceEnd,
                      bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.0f);
    }
#endif

    return bHit;
}

// ─────────────────────────────────────────────────────────────────────────────
// PROCESS HIT — Damage, surfaces, ragdoll impulse
// ─────────────────────────────────────────────────────────────────────────────
void ARealWorldWeaponBase::ProcessHit(ARealWorldCharacter* Instigator, const FHitResult& Hit)
{
    AActor* HitActor = Hit.GetActor();
    if (!HitActor) return;

    // Distance-based damage falloff
    const float Distance = FVector::Dist(GetActorLocation(), Hit.ImpactPoint);
    const float FalloffAlpha = FMath::Clamp(Distance / EffectiveRange, 0.0f, 1.0f);
    const float ActualDamage = FMath::Lerp(BaseDamage, BaseDamage * DamageFalloffMinMultiplier, FalloffAlpha);

    // Head-shot multiplier (bone name check)
    const float FinalDamage = (Hit.BoneName == TEXT("head") || Hit.BoneName == TEXT("neck_01"))
                            ? ActualDamage * HeadshotMultiplier
                            : ActualDamage;

    // Apply UE damage event
    FPointDamageEvent DamageEvent;
    DamageEvent.HitInfo    = Hit;
    DamageEvent.ShotDirection = (Hit.ImpactPoint - GetActorLocation()).GetSafeNormal();
    DamageEvent.DamageTypeClass = DamageTypeClass;

    HitActor->TakeDamage(FinalDamage, DamageEvent, Instigator->GetController(), this);

    // Physical impulse for ragdoll
    if (USkeletalMeshComponent* SkelMesh = HitActor->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (SkelMesh->IsSimulatingPhysics())
        {
            const FVector Impulse = DamageEvent.ShotDirection * BulletImpulse;
            SkelMesh->AddImpulseAtLocation(Impulse, Hit.ImpactPoint, Hit.BoneName);
        }
    }

    // Surface-specific impact VFX
    SpawnImpactEffect(Hit);
}

// ─────────────────────────────────────────────────────────────────────────────
// SURFACE IMPACT VFX
// ─────────────────────────────────────────────────────────────────────────────
void ARealWorldWeaponBase::SpawnImpactEffect(const FHitResult& Hit) const
{
    if (!GetWorld()) return;

    const EPhysicalSurface SurfaceType = Hit.PhysMaterial.IsValid()
                                        ? Hit.PhysMaterial->SurfaceType.GetValue()
                                        : SurfaceType_Default;

    UNiagaraSystem* ChosenEffect = nullptr;
    switch (SurfaceType)
    {
        case RWSurface::Flesh:    ChosenEffect = ImpactFX_Flesh;    break;
        case RWSurface::Concrete: ChosenEffect = ImpactFX_Concrete; break;
        case RWSurface::Metal:    ChosenEffect = ImpactFX_Metal;    break;
        case RWSurface::Glass:    ChosenEffect = ImpactFX_Glass;    break;
        case RWSurface::Wood:     ChosenEffect = ImpactFX_Wood;     break;
        case RWSurface::Water:    ChosenEffect = ImpactFX_Water;    break;
        default:                  ChosenEffect = ImpactFX_Default;  break;
    }

    if (ChosenEffect)
    {
        const FRotator ImpactRotation = Hit.ImpactNormal.Rotation();
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), ChosenEffect, Hit.ImpactPoint, ImpactRotation,
            FVector::OneVector, true, true, ENCPoolMethod::AutoRelease
        );
    }

    // Spawn bullet decal
    if (BulletDecal && Hit.GetComponent())
    {
        UGameplayStatics::SpawnDecalAttached(
            BulletDecal, FVector(4.0f, 4.0f, 4.0f),
            Hit.GetComponent(), Hit.BoneName,
            Hit.ImpactPoint, Hit.ImpactNormal.Rotation(),
            EAttachLocation::KeepWorldPosition,
            15.0f // fade duration
        );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// MUZZLE FLASH & CASING
// ─────────────────────────────────────────────────────────────────────────────
void ARealWorldWeaponBase::SpawnMuzzleFlash() const
{
    if (MuzzleFlashFX && WeaponMesh)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            MuzzleFlashFX, WeaponMesh, MuzzleSocketName,
            FVector::ZeroVector, FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true, true, ENCPoolMethod::AutoRelease
        );
    }
}

void ARealWorldWeaponBase::SpawnCasingEject() const
{
    if (CasingEjectFX && WeaponMesh)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            CasingEjectFX, WeaponMesh, EjectSocketName,
            FVector::ZeroVector, FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true, true, ENCPoolMethod::AutoRelease
        );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AUDIO
// ─────────────────────────────────────────────────────────────────────────────
void ARealWorldWeaponBase::PlayFireSound() const
{
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    }
}

void ARealWorldWeaponBase::PlayDryFireSound() const
{
    if (DryFireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DryFireSound, GetActorLocation());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// RECOIL
// ─────────────────────────────────────────────────────────────────────────────
void ARealWorldWeaponBase::ApplyRecoilToController(ARealWorldCharacter* Instigator) const
{
    if (!Instigator) return;

    APlayerController* PC = Cast<APlayerController>(Instigator->GetController());
    if (!PC) return;

    // Build recoil from curve or fixed values
    const float VerticalRecoil   = FMath::RandRange(RecoilVerticalMin, RecoilVerticalMax);
    const float HorizontalRecoil = FMath::RandRange(-RecoilHorizontalMax, RecoilHorizontalMax);

    // Add input rotation (negative pitch = look up)
    PC->AddPitchInput(-VerticalRecoil);
    PC->AddYawInput(HorizontalRecoil);
}

void ARealWorldWeaponBase::ApplyCameraShake(ARealWorldCharacter* Instigator) const
{
    if (!Instigator || !FireCameraShake) return;

    APlayerController* PC = Cast<APlayerController>(Instigator->GetController());
    if (PC)
    {
        PC->ClientStartCameraShake(FireCameraShake, 1.0f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// RELOAD
// ─────────────────────────────────────────────────────────────────────────────
bool ARealWorldWeaponBase::Reload()
{
    if (CurrentAmmoInMag >= MagazineCapacity) return false; // Already full
    if (TotalReserveAmmo <= 0) return false;                 // No ammo left

    const int32 AmmoNeeded   = MagazineCapacity - CurrentAmmoInMag;
    const int32 AmmoToReload = FMath::Min(AmmoNeeded, TotalReserveAmmo);

    CurrentAmmoInMag   += AmmoToReload;
    TotalReserveAmmo   -= AmmoToReload;

    if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
    }

    if (ARealWorldCharacter* Char = Cast<ARealWorldCharacter>(GetOwner()))
    {
        if (CharacterReloadMontage)
        {
            Char->PlayAnimMontage(CharacterReloadMontage);
        }
    }

    OnWeaponReloaded.Broadcast(CurrentAmmoInMag, TotalReserveAmmo);
    return true;
}
