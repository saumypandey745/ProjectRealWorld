// ============================================================================
// RealWorldHealthComponent.h
// Project RealWorld — Universal Health/Armor/Ragdoll System
// Works on: Player Character, NPCs, Vehicles (generic damage receiver)
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "RealWorldHealthComponent.generated.h"

// ─── State Enums ──────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EHealthState : uint8
{
    Alive       UMETA(DisplayName = "Alive"),
    Downed      UMETA(DisplayName = "Downed (DBNO)"),  // Down But Not Out
    Dead        UMETA(DisplayName = "Dead"),
};

// ─── Delegates ────────────────────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged,
    float, NewHealth, float, MaxHealth, AActor*, Instigator);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageTaken,
    float, DamageAmount, const FDamageEvent&, DamageEvent, AActor*, DamageCauser);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeath,
    AActor*, KilledActor, AActor*, Killer);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthStateChanged,
    EHealthState, NewState);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREALWORLD_API URealWorldHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URealWorldHealthComponent();

    virtual void BeginPlay() override;

    // ─── Config ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Config")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Config")
    float MaxArmor = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Config")
    bool bCanDieFromDamage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Config")
    bool bHasDownedState = false; // GTA Online-style DBNO

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Config")
    float RagdollImpulseThreshold = 10000.0f; // Min impulse to trigger ragdoll

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Config")
    float HealthRegenDelay = 6.0f;  // Seconds after taking damage before regen starts

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Config")
    float HealthRegenRate = 5.0f;   // HP/s (0 = disabled)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Config")
    float ArmorDamageAbsorption = 0.5f; // 50% of damage absorbed by armor

    // ─── State ───────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|State")
    float CurrentHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|State")
    float CurrentArmor = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|State")
    EHealthState HealthState = EHealthState::Alive;

    // ─── VFX/SFX ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|VFX")
    TObjectPtr<UNiagaraSystem> BloodSplatterFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|VFX")
    TObjectPtr<UNiagaraSystem> BloodPoolFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|SFX")
    TObjectPtr<USoundBase> PainSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|SFX")
    TObjectPtr<USoundBase> DeathSound;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Health|Actions")
    float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                     AController* EventInstigator, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Health|Actions")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Health|Actions")
    void AddArmor(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Health|Actions")
    void Kill(AActor* Killer = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Health|Actions")
    void Revive(float ReviveHealth = 50.0f);

    // ─── Queries ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "Health|State")
    bool IsAlive() const { return HealthState == EHealthState::Alive; }

    UFUNCTION(BlueprintPure, Category = "Health|State")
    bool IsDead() const { return HealthState == EHealthState::Dead; }

    UFUNCTION(BlueprintPure, Category = "Health|State")
    float GetHealthPercent() const { return CurrentHealth / MaxHealth; }

    UFUNCTION(BlueprintPure, Category = "Health|State")
    float GetArmorPercent() const { return MaxArmor > 0.0f ? CurrentArmor / MaxArmor : 0.0f; }

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnDamageTaken OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnDeath OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnHealthStateChanged OnHealthStateChanged;

private:
    float LastDamageTime = 0.0f;
    FTimerHandle RegenTimerHandle;

    void StartRegenTimer();
    void TickRegen();
    void TriggerRagdoll(const FVector& ImpactImpulse, const FName& HitBone);
    void SpawnBloodVFX(const FVector& Location, const FVector& Direction) const;
    void ChangeHealthState(EHealthState NewState);
};
