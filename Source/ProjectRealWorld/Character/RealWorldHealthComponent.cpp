// ============================================================================
// RealWorldHealthComponent.cpp
// Project RealWorld — Universal Health / Armor / Ragdoll System
// ============================================================================

#include "Character/RealWorldHealthComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "TimerManager.h"

URealWorldHealthComponent::URealWorldHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URealWorldHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    CurrentArmor  = 0.0f;

    // Hook into owner's TakeDamage event
    if (AActor* Owner = GetOwner())
    {
        Owner->OnTakeAnyDamage.AddDynamic(this, &URealWorldHealthComponent::TakeDamage_Internal);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper to receive UE's TakeDamage broadcast
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldHealthComponent::TakeDamage_Internal(AActor* DamagedActor, float Damage,
    const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    FDamageEvent DmgEvent;
    TakeDamage(Damage, DmgEvent, InstigatedBy, DamageCauser);
}

// ─────────────────────────────────────────────────────────────────────────────
// TAKE DAMAGE
// ─────────────────────────────────────────────────────────────────────────────
float URealWorldHealthComponent::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                                             AController* EventInstigator, AActor* DamageCauser)
{
    if (!IsAlive() || DamageAmount <= 0.0f) return 0.0f;

    float ActualDamage = DamageAmount;

    // ── Armor absorption ──────────────────────────────────────────────────────
    if (CurrentArmor > 0.0f)
    {
        const float ArmorAbsorb = FMath::Min(ActualDamage * ArmorDamageAbsorption, CurrentArmor);
        CurrentArmor  -= ArmorAbsorb;
        ActualDamage  -= ArmorAbsorb;
    }

    CurrentHealth = FMath::Max(CurrentHealth - ActualDamage, 0.0f);
    LastDamageTime = GetWorld()->GetTimeSeconds();

    // ── Blood VFX ─────────────────────────────────────────────────────────────
    FVector ImpactPoint  = GetOwner()->GetActorLocation();
    FVector ImpactNormal = DamageCauser ? (ImpactPoint - DamageCauser->GetActorLocation()).GetSafeNormal()
                                        : FVector::UpVector;

    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        const FPointDamageEvent& PointEvent = static_cast<const FPointDamageEvent&>(DamageEvent);
        ImpactPoint  = PointEvent.HitInfo.ImpactPoint;
        ImpactNormal = PointEvent.HitInfo.ImpactNormal;
    }

    SpawnBloodVFX(ImpactPoint, ImpactNormal);

    // ── Pain SFX ──────────────────────────────────────────────────────────────
    if (PainSound && FMath::RandBool())
    {
        UGameplayStatics::PlaySoundAtLocation(this, PainSound, ImpactPoint);
    }

    OnDamageTaken.Broadcast(ActualDamage, DamageEvent, DamageCauser);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth,
                               EventInstigator ? EventInstigator->GetPawn() : nullptr);

    // ── Death check ───────────────────────────────────────────────────────────
    if (CurrentHealth <= 0.0f && bCanDieFromDamage)
    {
        if (bHasDownedState && HealthState == EHealthState::Alive)
        {
            // DBNO — go down first
            ChangeHealthState(EHealthState::Downed);
        }
        else
        {
            Kill(DamageCauser);
        }
    }
    else
    {
        // Start regen timer on damage
        StartRegenTimer();
    }

    return ActualDamage;
}

// ─────────────────────────────────────────────────────────────────────────────
// KILL
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldHealthComponent::Kill(AActor* Killer)
{
    if (HealthState == EHealthState::Dead) return;

    CurrentHealth = 0.0f;
    ChangeHealthState(EHealthState::Dead);

    // Death SFX
    if (DeathSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetOwner()->GetActorLocation());
    }

    // Blood pool VFX
    if (BloodPoolFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), BloodPoolFX, GetOwner()->GetActorLocation(),
            FRotator::ZeroRotator, FVector::OneVector,
            true, true, ENCPoolMethod::None
        );
    }

    // Trigger ragdoll on character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        // Enable physics on full skeleton
        Character->GetMesh()->SetSimulatePhysics(true);
        Character->GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
        Character->GetCharacterMovement()->DisableMovement();
        Character->GetCharacterMovement()->StopMovementImmediately();

        // Apply death impulse
        Character->GetMesh()->AddImpulse(
            FVector(FMath::RandRange(-100.0f, 100.0f),
                    FMath::RandRange(-100.0f, 100.0f),
                    FMath::RandRange(200.0f, 500.0f)),
            TEXT("pelvis"),
            true
        );

        // Disable capsule collision
        Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    OnDeath.Broadcast(GetOwner(), Killer);

    // Schedule body cleanup (GTA-style: body stays for 30s then fades)
    if (GetOwner())
    {
        GetOwner()->SetLifeSpan(30.0f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// HEAL
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldHealthComponent::Heal(float Amount)
{
    if (!IsAlive() || Amount <= 0.0f) return;

    CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, nullptr);
}

void URealWorldHealthComponent::AddArmor(float Amount)
{
    CurrentArmor = FMath::Min(CurrentArmor + Amount, MaxArmor);
}

// ─────────────────────────────────────────────────────────────────────────────
// REVIVE (DBNO recovery)
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldHealthComponent::Revive(float ReviveHealth)
{
    if (HealthState == EHealthState::Dead) return;

    CurrentHealth = FMath::Clamp(ReviveHealth, 1.0f, MaxHealth);
    ChangeHealthState(EHealthState::Alive);

    // Stand character back up (disable ragdoll)
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        Character->GetMesh()->SetSimulatePhysics(false);
        Character->GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
        Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
    }

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, nullptr);
}

// ─────────────────────────────────────────────────────────────────────────────
// HEALTH REGEN
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldHealthComponent::StartRegenTimer()
{
    if (HealthRegenRate <= 0.0f) return;

    GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(
        RegenTimerHandle,
        this, &URealWorldHealthComponent::TickRegen,
        0.5f,  // Every 0.5s check
        true,  // Looping
        HealthRegenDelay // Initial delay
    );
}

void URealWorldHealthComponent::TickRegen()
{
    if (!IsAlive() || CurrentHealth >= MaxHealth)
    {
        GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);
        return;
    }

    Heal(HealthRegenRate * 0.5f); // 0.5s interval × rate
}

// ─────────────────────────────────────────────────────────────────────────────
// RAGDOLL
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldHealthComponent::TriggerRagdoll(const FVector& ImpactImpulse, const FName& HitBone)
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;

    // Only ragdoll if impulse above threshold
    if (ImpactImpulse.SizeSquared() < RagdollImpulseThreshold * RagdollImpulseThreshold) return;

    Character->GetMesh()->SetSimulatePhysics(true);
    Character->GetMesh()->AddImpulseAtLocation(ImpactImpulse, Character->GetActorLocation(), HitBone);
}

void URealWorldHealthComponent::ApplyRagdollImpulse(const FVector& Impulse, const FName& HitBone)
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;

    if (Character->GetMesh())
    {
        Character->GetMesh()->SetSimulatePhysics(true);
        Character->GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
        Character->GetMesh()->AddImpulse(Impulse, HitBone, true);
    }
    if (Character->GetCapsuleComponent())
    {
        Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (Character->GetCharacterMovement())
    {
        Character->GetCharacterMovement()->DisableMovement();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// BLOOD VFX
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldHealthComponent::SpawnBloodVFX(const FVector& Location, const FVector& Direction) const
{
    if (BloodSplatterFX)
    {
        const FRotator BloodRotation = Direction.Rotation();
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), BloodSplatterFX, Location, BloodRotation,
            FVector::OneVector, true, true, ENCPoolMethod::AutoRelease
        );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// STATE CHANGE
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldHealthComponent::ChangeHealthState(EHealthState NewState)
{
    if (HealthState == NewState) return;
    HealthState = NewState;
    OnHealthStateChanged.Broadcast(NewState);
}
