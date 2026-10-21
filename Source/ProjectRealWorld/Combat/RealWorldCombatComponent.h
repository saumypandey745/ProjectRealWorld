#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/RealWorldWeaponBase.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "RealWorldCombatComponent.generated.h"

class ARealWorldCharacter;
class UNiagaraSystem;

// ─── Delegates ───────────────────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEquipped,     ARealWorldWeaponBase*, NewWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponAmmoChanged, int32, CurrentAmmo, int32, ReserveAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeleeAttack,        int32, ComboStep);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTakedownExecuted,   AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoverStateChanged,  bool, bEnteredCover);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREALWORLD_API URealWorldCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URealWorldCombatComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                                FActorComponentTickFunction* ThisTickFunction) override;

    // ─── State ───────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    TObjectPtr<ARealWorldWeaponBase> CurrentWeapon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    bool bIsInCover = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    bool bIsAiming = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    bool bIsInCombat = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    bool bIsInVehicle = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    bool bIsPeeking = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Stamina")
    float CurrentStamina = 100.0f;

    // ─── Melee Config ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
    float MeleeBaseDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
    float MeleeRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
    float MeleeRadius = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
    float MeleeKnockbackForce = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
    int32 MaxComboSteps = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
    float ComboWindowDuration = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
    float ComboDamageEscalation = 0.35f; // +35% per combo step

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee")
    float MeleeStaminaCost = 10.0f;

    // ─── Takedown Config ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Takedown")
    float TakedownRange = 180.0f;

    // ─── Cover Config ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Cover")
    float CoverDetectionRange = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Cover")
    float CoverPeekDistance = 80.0f;

    // ─── Stamina Config ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina")
    float StaminaRegenRate = 15.0f;

    // ─── Aim Config ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Aim")
    float DefaultWalkSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Aim")
    float AimWalkSpeed = 250.0f;

    // ─── Mobile Aim Assist ───────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|AimAssist")
    bool bAimAssistEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|AimAssist")
    float AimAssistRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|AimAssist")
    float AimAssistStrength = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|AimAssist")
    float AimAssistMaxAngle = 15.0f; // degrees

    // ─── VFX / SFX ───────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee|VFX")
    TObjectPtr<UNiagaraSystem> MeleeImpactFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee|SFX")
    TObjectPtr<USoundBase> MeleeSwingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Melee|SFX")
    TObjectPtr<USoundBase> MeleeHitSound;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    void EquipWeapon(TSubclassOf<ARealWorldWeaponBase> WeaponClass);

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    void UnequipWeapon();

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    bool FireCurrentWeapon();

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    bool ReloadCurrentWeapon();

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    void SetAiming(bool bNewAiming);

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    bool PerformMeleeAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    bool TryEnterCover();

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    void ExitCover();

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    bool CanPeekFromCover() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    void PeekFromCover(bool bRight);

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    void ReturnToCover();

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnWeaponEquipped OnWeaponEquipped;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnWeaponAmmoChanged OnWeaponAmmoChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnMeleeAttack OnMeleeAttackPerformed;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnTakedownExecuted OnTakedownExecuted;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnCoverStateChanged OnCoverStateChanged;

private:
    UPROPERTY()
    TObjectPtr<ARealWorldCharacter> OwnerCharacter;

    // Cover state
    FVector CoverNormal       = FVector::ZeroVector;
    FVector CoverAttachPoint  = FVector::ZeroVector;

    // Melee combo state
    int32 CurrentComboStep    = 0;
    bool  bComboWindowOpen    = false;
    float ComboWindowTimer    = 0.0f;

    // Aim assist
    UPROPERTY()
    TObjectPtr<AActor> AimAssistTarget;

    // Methods
    bool PerformMeleeTrace(FHitResult& OutHit) const;
    void ResetCombo();
    bool CanExecuteTakedown(AActor* TargetActor) const;
    void ExecuteTakedown(AActor* TargetActor);
    void TickAimAssist(float DeltaTime);
    AActor* FindBestAimAssistTarget() const;

    UFUNCTION()
    void OnCurrentWeaponFired(int32 CurrentAmmo, int32 ReserveAmmo);
};
