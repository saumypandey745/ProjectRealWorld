#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "RealWorldWeaponBase.generated.h"

class USkeletalMeshComponent;
class ARealWorldCharacter;
class UNiagaraSystem;
class USoundBase;
class UMaterialInterface;
class UCameraShakeBase;
class UDamageType;

UENUM(BlueprintType)
enum class EWeaponClass : uint8
{
    Melee         UMETA(DisplayName = "Melee"),
    Pistol        UMETA(DisplayName = "Pistol"),
    SMG           UMETA(DisplayName = "Submachine Gun"),
    AssaultRifle  UMETA(DisplayName = "Assault Rifle"),
    Shotgun       UMETA(DisplayName = "Shotgun"),
    SniperRifle   UMETA(DisplayName = "Sniper Rifle")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponFired,   int32, CurrentAmmo, int32, ReserveAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponReloaded, int32, CurrentAmmo, int32, ReserveAmmo);

UCLASS(Abstract, BlueprintType, Blueprintable)
class PROJECTREALWORLD_API ARealWorldWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    ARealWorldWeaponBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Mesh ────────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Mesh")
    TObjectPtr<USkeletalMeshComponent> WeaponMesh;

    // ─── Config ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config")
    EWeaponClass WeaponClass = EWeaponClass::Pistol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config")
    FName MuzzleSocketName = TEXT("MuzzleFlash");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config")
    FName EjectSocketName = TEXT("CasingEject");

    // ─── Stats ───────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float BaseDamage = 32.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float FireRateRPM = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float EffectiveRange = 4500.0f; // cm (45 meters)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float BaseSpreadDegrees = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float HeadshotMultiplier = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float DamageFalloffMinMultiplier = 0.3f; // Min damage at max range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float BulletImpulse = 50000.0f; // Impulse applied to ragdoll

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats", meta = (ClampMin = "0"))
    TSubclassOf<UDamageType> DamageTypeClass;

    // ─── Ammo ────────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
    int32 MagazineCapacity = 15;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
    int32 CurrentAmmoInMag = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
    int32 TotalReserveAmmo = 60;

    // ─── Recoil ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float RecoilVerticalMin = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float RecoilVerticalMax = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float RecoilHorizontalMax = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float RecoilRecoverySpeed = 8.0f;

    // ─── VFX ─────────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    TObjectPtr<UNiagaraSystem> MuzzleFlashFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    TObjectPtr<UNiagaraSystem> CasingEjectFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    TObjectPtr<UNiagaraSystem> ImpactFX_Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    TObjectPtr<UNiagaraSystem> ImpactFX_Flesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    TObjectPtr<UNiagaraSystem> ImpactFX_Concrete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    TObjectPtr<UNiagaraSystem> ImpactFX_Metal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    TObjectPtr<UNiagaraSystem> ImpactFX_Glass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    TObjectPtr<UNiagaraSystem> ImpactFX_Wood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    TObjectPtr<UNiagaraSystem> ImpactFX_Water;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    TObjectPtr<UMaterialInterface> BulletDecal;

    // ─── SFX ─────────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|SFX")
    TObjectPtr<USoundBase> FireSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|SFX")
    TObjectPtr<USoundBase> DryFireSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|SFX")
    TObjectPtr<USoundBase> ReloadSound;

    // ─── Camera Shake ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Feel")
    TSubclassOf<UCameraShakeBase> FireCameraShake;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Weapon|Actions")
    virtual bool FireWeapon(ARealWorldCharacter* InstigatorPawn);

    UFUNCTION(BlueprintCallable, Category = "Weapon|Actions")
    virtual bool Reload();

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
    FOnWeaponFired    OnWeaponFired;

    UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
    FOnWeaponReloaded OnWeaponReloaded;

protected:
    float LastFireTime = 0.0f;
    FVector2D CurrentRecoilOffset = FVector2D::ZeroVector;

    bool PerformBallisticTrace(ARealWorldCharacter* Instigator, FHitResult& OutHit) const;
    void ProcessHit(ARealWorldCharacter* Instigator, const FHitResult& Hit);
    void SpawnImpactEffect(const FHitResult& Hit) const;
    void SpawnMuzzleFlash() const;
    void SpawnCasingEject() const;
    void PlayFireSound() const;
    void PlayDryFireSound() const;
    void ApplyRecoilToController(ARealWorldCharacter* Instigator) const;
    void ApplyCameraShake(ARealWorldCharacter* Instigator) const;

#if WITH_EDITOR
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebugTrace = false;
#endif
};
