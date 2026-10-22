// ============================================================================
// RealWorldVehicleDamageComponent.h
// Project RealWorld — GTA5-class Vehicle Damage & Deformation System
// Handles: procedural mesh deformation, engine/tyre/glass damage states,
//          fire propagation, explosion, component breakage
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "RealWorldVehicleDamageComponent.generated.h"

class ARealWorldVehicleBase;
class UDestructibleComponent;

// ─── Damage States ────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EVehicleDamageZone : uint8
{
    FrontLeft   UMETA(DisplayName = "Front Left"),
    FrontRight  UMETA(DisplayName = "Front Right"),
    RearLeft    UMETA(DisplayName = "Rear Left"),
    RearRight   UMETA(DisplayName = "Rear Right"),
    Hood        UMETA(DisplayName = "Hood"),
    Trunk       UMETA(DisplayName = "Trunk"),
    Roof        UMETA(DisplayName = "Roof"),
};

UENUM(BlueprintType)
enum class EEngineState : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Damaged     UMETA(DisplayName = "Damaged (Smoking)"),
    Critical    UMETA(DisplayName = "Critical (Fire)"),
    Destroyed   UMETA(DisplayName = "Destroyed"),
};

UENUM(BlueprintType)
enum class ETyreState : uint8
{
    Inflated    UMETA(DisplayName = "Inflated"),
    Deflated    UMETA(DisplayName = "Punctured / Flat"),
};

USTRUCT(BlueprintType)
struct FVehicleDamageState
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float BodyIntegrity = 1.0f;       // 0-1 (1 = pristine)

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EEngineState EngineState = EEngineState::Normal;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<ETyreState> TyreStates;    // Index: FL, FR, RL, RR

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWindowsFront = false;       // Windows broken?

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWindowsRear = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsOnFire = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float FireTimer = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVehicleExploded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEngineStateChanged, EEngineState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTyreDamaged, int32, TyreIndex, ETyreState, NewState);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREALWORLD_API URealWorldVehicleDamageComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URealWorldVehicleDamageComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                                FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Config ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|Config")
    float MaxHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|Config")
    float FireIgnitionThreshold = 200.0f;   // Health below this = can catch fire

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|Config")
    float FireDamagePerSecond = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|Config")
    float ExplosionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|Config")
    float ExplosionDamage = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|Config")
    float DeformationScale = 1.0f; // Multiplier for visual deformation intensity

    // ─── State ───────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VehicleDamage|State")
    float CurrentHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VehicleDamage|State")
    FVehicleDamageState DamageState;

    // ─── VFX Assets ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|VFX")
    TObjectPtr<UNiagaraSystem> EngineSmokeLight_FX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|VFX")
    TObjectPtr<UNiagaraSystem> EngineFireFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|VFX")
    TObjectPtr<UNiagaraSystem> ExplosionFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|VFX")
    TObjectPtr<UNiagaraSystem> SparksOnImpactFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|VFX")
    TObjectPtr<UNiagaraSystem> TyreSmokeFL_FX;

    // ─── SFX Assets ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|SFX")
    TObjectPtr<USoundBase> ImpactMetalSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|SFX")
    TObjectPtr<USoundBase> TyreBurstSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|SFX")
    TObjectPtr<USoundBase> ExplosionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VehicleDamage|SFX")
    TObjectPtr<USoundBase> FireLoopSound;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "VehicleDamage|Actions")
    void ApplyImpactDamage(float Damage, const FVector& ImpactPoint, const FVector& ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "VehicleDamage|Actions")
    void PunctureRandomTyre();

    UFUNCTION(BlueprintCallable, Category = "VehicleDamage|Actions")
    void PunctureTyre(int32 TyreIndex);

    UFUNCTION(BlueprintCallable, Category = "VehicleDamage|Actions")
    void IgniteFire();

    UFUNCTION(BlueprintCallable, Category = "VehicleDamage|Actions")
    void Explode();

    UFUNCTION(BlueprintCallable, Category = "VehicleDamage|Actions")
    void Repair(float Amount);

    UFUNCTION(BlueprintCallable, Category = "VehicleDamage|Actions")
    void RepairVehicle();

    UFUNCTION(BlueprintPure, Category = "VehicleDamage|State")
    float GetHealthPercent() const { return CurrentHealth / MaxHealth; }

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "VehicleDamage|Events")
    FOnVehicleExploded OnVehicleExploded;

    UPROPERTY(BlueprintAssignable, Category = "VehicleDamage|Events")
    FOnEngineStateChanged OnEngineStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "VehicleDamage|Events")
    FOnTyreDamaged OnTyreDamaged;

private:
    UPROPERTY()
    TObjectPtr<ARealWorldVehicleBase> OwnerVehicle;

    bool bExploded = false;

    // Active VFX components
    UPROPERTY()
    TObjectPtr<UNiagaraComponent> ActiveSmokeFX;

    UPROPERTY()
    TObjectPtr<UNiagaraComponent> ActiveFireFX;

    void UpdateEngineState();
    void TickFireDamage(float DeltaTime);
    void DeformBodyAtPoint(const FVector& WorldPoint, float DamageAmount);
    EVehicleDamageZone ClassifyImpactZone(const FVector& LocalImpactPoint) const;
    void SpawnImpactSparks(const FVector& ImpactPoint) const;
};
