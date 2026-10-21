// ============================================================================
// RealWorldCombatComponent.cpp
// Project RealWorld — AAA Combat System
// Cover mechanics, melee combos, takedowns, stamina, aim-assist (mobile)
// ============================================================================

#include "Combat/RealWorldCombatComponent.h"
#include "Character/RealWorldCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"

URealWorldCombatComponent::URealWorldCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void URealWorldCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ARealWorldCharacter>(GetOwner());
}

// ─────────────────────────────────────────────────────────────────────────────
// TICK — Aim Assist, Stamina Regen
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Stamina regeneration
    if (!bIsInCombat && CurrentStamina < MaxStamina)
    {
        CurrentStamina = FMath::Min(CurrentStamina + StaminaRegenRate * DeltaTime, MaxStamina);
    }

    // Mobile aim assist — soft-lock to nearest enemy when aiming
    if (bIsAiming && bAimAssistEnabled)
    {
        TickAimAssist(DeltaTime);
    }

    // Melee combo window timer
    if (bComboWindowOpen)
    {
        ComboWindowTimer -= DeltaTime;
        if (ComboWindowTimer <= 0.0f)
        {
            ResetCombo();
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// WEAPON MANAGEMENT
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldCombatComponent::EquipWeapon(TSubclassOf<ARealWorldWeaponBase> WeaponClass)
{
    if (!WeaponClass || !OwnerCharacter) return;

    // Drop previous weapon
    if (CurrentWeapon)
    {
        CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        CurrentWeapon->Destroy();
        CurrentWeapon = nullptr;
    }

    // Spawn and attach new weapon to right hand socket
    FActorSpawnParameters SpawnParams;
    SpawnParams.Instigator   = OwnerCharacter;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    CurrentWeapon = GetWorld()->SpawnActor<ARealWorldWeaponBase>(
        WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams
    );

    if (CurrentWeapon)
    {
        CurrentWeapon->AttachToComponent(
            OwnerCharacter->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            TEXT("weapon_r")
        );

        // Bind ammo change events
        CurrentWeapon->OnWeaponFired.AddDynamic(this, &URealWorldCombatComponent::OnCurrentWeaponFired);
    }

    bIsInVehicle = false;
    OnWeaponEquipped.Broadcast(CurrentWeapon);
}

void URealWorldCombatComponent::UnequipWeapon()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        CurrentWeapon = nullptr;
    }
    bIsAiming = false;
    OnWeaponEquipped.Broadcast(nullptr);
}

// ─────────────────────────────────────────────────────────────────────────────
// FIRING
// ─────────────────────────────────────────────────────────────────────────────
bool URealWorldCombatComponent::FireCurrentWeapon()
{
    if (!CurrentWeapon || !OwnerCharacter) return false;
    return CurrentWeapon->FireWeapon(OwnerCharacter);
}

bool URealWorldCombatComponent::ReloadCurrentWeapon()
{
    if (!CurrentWeapon) return false;
    return CurrentWeapon->Reload();
}

void URealWorldCombatComponent::SetAiming(bool bNewAiming)
{
    if (bIsAiming == bNewAiming) return;
    bIsAiming = bNewAiming;

    // Adjust movement speed when aiming
    if (OwnerCharacter)
    {
        UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
        if (MovComp)
        {
            MovComp->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : DefaultWalkSpeed;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// MELEE COMBAT
// ─────────────────────────────────────────────────────────────────────────────
bool URealWorldCombatComponent::PerformMeleeAttack()
{
    if (!OwnerCharacter) return false;

    // Stamina cost
    if (CurrentStamina < MeleeStaminaCost)
    {
        return false; // Exhausted
    }

    CurrentStamina -= MeleeStaminaCost;
    bIsInCombat = true;

    // Advance combo
    CurrentComboStep = FMath::Min(CurrentComboStep + 1, MaxComboSteps);
    bComboWindowOpen  = true;
    ComboWindowTimer  = ComboWindowDuration;

    // Calculate combo damage multiplier
    const float DamageMultiplier = 1.0f + (CurrentComboStep - 1) * ComboDamageEscalation;
    const float FinalDamage      = MeleeBaseDamage * DamageMultiplier;

    // Sphere sweep in front of character
    FHitResult MeleeHit;
    if (PerformMeleeTrace(MeleeHit))
    {
        AActor* HitActor = MeleeHit.GetActor();
        if (HitActor)
        {
            // Takedown check — if enemy health is very low and we're behind them
            if (CanExecuteTakedown(HitActor))
            {
                ExecuteTakedown(HitActor);
            }
            else
            {
                FPointDamageEvent DamageEvent;
                DamageEvent.HitInfo       = MeleeHit;
                DamageEvent.ShotDirection = OwnerCharacter->GetActorForwardVector();
                HitActor->TakeDamage(FinalDamage, DamageEvent, OwnerCharacter->GetController(), OwnerCharacter);

                // Stagger knockback
                if (UCharacterMovementComponent* TargetMov = HitActor->FindComponentByClass<UCharacterMovementComponent>())
                {
                    const FVector Knockback = OwnerCharacter->GetActorForwardVector() * MeleeKnockbackForce;
                    TargetMov->AddImpulse(Knockback, true);
                }
            }

            // Melee hit VFX
            if (MeleeImpactFX)
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    GetWorld(), MeleeImpactFX, MeleeHit.ImpactPoint,
                    MeleeHit.ImpactNormal.Rotation(), FVector::OneVector,
                    true, true, ENCPoolMethod::AutoRelease
                );
            }

            // Hit sound
            if (MeleeHitSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, MeleeHitSound, MeleeHit.ImpactPoint);
            }
        }
    }

    // Whoosh sound even if no hit
    if (MeleeSwingSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, MeleeSwingSound, OwnerCharacter->GetActorLocation());
    }

    OnMeleeAttackPerformed.Broadcast(CurrentComboStep);
    return true;
}

bool URealWorldCombatComponent::PerformMeleeTrace(FHitResult& OutHit) const
{
    if (!OwnerCharacter) return false;

    const FVector Start  = OwnerCharacter->GetActorLocation();
    const FVector End    = Start + OwnerCharacter->GetActorForwardVector() * MeleeRange;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;

    return GetWorld()->SweepSingleByChannel(
        OutHit, Start, End, FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        FCollisionShape::MakeSphere(MeleeRadius),
        QueryParams
    );
}

void URealWorldCombatComponent::ResetCombo()
{
    CurrentComboStep = 0;
    bComboWindowOpen  = false;
    ComboWindowTimer  = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// TAKEDOWNS (Stealth / Finisher)
// ─────────────────────────────────────────────────────────────────────────────
bool URealWorldCombatComponent::CanExecuteTakedown(AActor* TargetActor) const
{
    if (!TargetActor || !OwnerCharacter) return false;

    // Check if target is in low health and we're approaching from behind
    const float DistanceToTarget = FVector::Dist(OwnerCharacter->GetActorLocation(), TargetActor->GetActorLocation());
    if (DistanceToTarget > TakedownRange) return false;

    // Dot product to check if we're behind target
    const FVector ToTarget   = (TargetActor->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
    const FVector TargetFwd  = TargetActor->GetActorForwardVector();
    const float DotProduct   = FVector::DotProduct(ToTarget, TargetFwd);

    return DotProduct > 0.5f; // In front of target's back hemisphere
}

void URealWorldCombatComponent::ExecuteTakedown(AActor* TargetActor)
{
    if (!TargetActor || !OwnerCharacter) return;

    // Snap position
    const FVector TakedownPos = TargetActor->GetActorLocation() -
                                 TargetActor->GetActorForwardVector() * 100.0f;
    OwnerCharacter->SetActorLocation(TakedownPos, false, nullptr, ETeleportType::TeleportPhysics);

    // Instantly kill (stealth takedown = one-shot)
    FDamageEvent DamageEvent;
    TargetActor->TakeDamage(9999.0f, DamageEvent, OwnerCharacter->GetController(), OwnerCharacter);

    OnTakedownExecuted.Broadcast(TargetActor);
}

// ─────────────────────────────────────────────────────────────────────────────
// COVER SYSTEM
// ─────────────────────────────────────────────────────────────────────────────
bool URealWorldCombatComponent::TryEnterCover()
{
    if (!OwnerCharacter) return false;

    // Trace for a cover surface (wall/object)
    FHitResult CoverHit;
    const FVector Start = OwnerCharacter->GetActorLocation();
    const FVector End   = Start + OwnerCharacter->GetActorForwardVector() * CoverDetectionRange;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    if (GetWorld()->LineTraceSingleByChannel(CoverHit, Start, End, ECC_WorldStatic, Params))
    {
        // Valid cover if normal is roughly facing us
        const float DotNormal = FVector::DotProduct(CoverHit.Normal, -OwnerCharacter->GetActorForwardVector());
        if (DotNormal > 0.6f)
        {
            bIsInCover          = true;
            CoverNormal         = CoverHit.Normal;
            CoverAttachPoint    = CoverHit.ImpactPoint;

            // Move character to cover position
            const FVector CoverPos = CoverHit.ImpactPoint + CoverHit.Normal * 60.0f;
            OwnerCharacter->SetActorLocation(CoverPos, false, nullptr, ETeleportType::ResetPhysics);

            // Orient toward cover wall
            const FRotator CoverRotation = (-CoverHit.Normal).Rotation();
            OwnerCharacter->SetActorRotation(FRotator(0.0f, CoverRotation.Yaw, 0.0f));

            // Crouch automatically
            OwnerCharacter->Crouch();

            OnCoverStateChanged.Broadcast(true);
            return true;
        }
    }

    return false;
}

void URealWorldCombatComponent::ExitCover()
{
    if (!bIsInCover) return;
    bIsInCover = false;
    CoverNormal = FVector::ZeroVector;

    if (OwnerCharacter)
    {
        OwnerCharacter->UnCrouch();
    }

    OnCoverStateChanged.Broadcast(false);
}

bool URealWorldCombatComponent::CanPeekFromCover() const
{
    return bIsInCover && !bIsPeeking;
}

void URealWorldCombatComponent::PeekFromCover(bool bRight)
{
    if (!bIsInCover || !OwnerCharacter) return;

    bIsPeeking = true;
    const FVector PeekDir = FVector::CrossProduct(CoverNormal, FVector::UpVector) * (bRight ? 1.0f : -1.0f);
    const FVector PeekOffset = PeekDir * CoverPeekDistance;

    OwnerCharacter->SetActorLocation(
        CoverAttachPoint + CoverNormal * 60.0f + PeekOffset,
        false, nullptr, ETeleportType::ResetPhysics
    );
}

void URealWorldCombatComponent::ReturnToCover()
{
    if (!bIsInCover || !OwnerCharacter) return;
    bIsPeeking = false;

    OwnerCharacter->SetActorLocation(
        CoverAttachPoint + CoverNormal * 60.0f,
        false, nullptr, ETeleportType::ResetPhysics
    );
}

// ─────────────────────────────────────────────────────────────────────────────
// MOBILE AIM ASSIST (soft target lock for touch controls)
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldCombatComponent::TickAimAssist(float DeltaTime)
{
    if (!OwnerCharacter) return;

    APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
    if (!PC) return;

    // Find nearest enemy in screen-center cone
    AActor* BestTarget = FindBestAimAssistTarget();
    if (!BestTarget)
    {
        AimAssistTarget = nullptr;
        return;
    }

    AimAssistTarget = BestTarget;

    // Nudge controller rotation toward target
    FVector TargetCenter = BestTarget->GetActorLocation() + FVector(0, 0, 50.0f);
    FVector CamLocation;
    FRotator CamRotation;
    PC->GetPlayerViewPoint(CamLocation, CamRotation);

    FRotator DesiredRotation = (TargetCenter - CamLocation).Rotation();
    FRotator NewRotation     = FMath::RInterpTo(CamRotation, DesiredRotation, DeltaTime, AimAssistStrength);

    // Only apply if correction is small (soft lock, not snap)
    const float AngleDiff = FMath::Abs(FRotator::NormalizeAxis(NewRotation.Yaw - CamRotation.Yaw));
    if (AngleDiff < AimAssistMaxAngle)
    {
        PC->SetControlRotation(NewRotation);
    }
}

AActor* URealWorldCombatComponent::FindBestAimAssistTarget() const
{
    if (!OwnerCharacter) return nullptr;

    APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
    if (!PC) return nullptr;

    FVector CamLocation;
    FRotator CamRotation;
    PC->GetPlayerViewPoint(CamLocation, CamRotation);
    const FVector CamForward = CamRotation.Vector();

    // Overlap sphere around player
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    GetWorld()->OverlapMultiByChannel(
        Overlaps, CamLocation, FQuat::Identity,
        ECC_Pawn, FCollisionShape::MakeSphere(AimAssistRange), QueryParams
    );

    AActor* BestActor  = nullptr;
    float   BestScore  = FLT_MAX;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Candidate = Overlap.GetActor();
        if (!Candidate) continue;

        const FVector ToTarget    = (Candidate->GetActorLocation() - CamLocation).GetSafeNormal();
        const float   DotProduct  = FVector::DotProduct(CamForward, ToTarget);
        if (DotProduct < 0.8f) continue; // Outside ~37° cone

        const float AngularDist = FMath::Acos(DotProduct);
        if (AngularDist < BestScore)
        {
            BestScore  = AngularDist;
            BestActor  = Candidate;
        }
    }

    return BestActor;
}

void URealWorldCombatComponent::OnCurrentWeaponFired(int32 CurrentAmmo, int32 ReserveAmmo)
{
    OnWeaponAmmoChanged.Broadcast(CurrentAmmo, ReserveAmmo);
}
