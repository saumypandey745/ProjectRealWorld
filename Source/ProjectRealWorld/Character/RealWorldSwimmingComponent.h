// ============================================================================
// RealWorldSwimmingComponent.h
// Project RealWorld — GTA V-Class Swimming & Deep-Water Diving System
// Features: Surface breaststroke, underwater 3D diving, oxygen capacity,
//           drowning damage, water surface breakout
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "NiagaraSystem.h"
#include "RealWorldSwimmingComponent.generated.h"

class ACharacter;
class URealWorldHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubmergedStateChanged, bool, bIsUnderwater);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOxygenChanged, float, CurrentOxygen, float, MaxOxygen);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREALWORLD_API URealWorldSwimmingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URealWorldSwimmingComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                                FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Config ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Config")
    float WaterSurfaceZ = 0.0f;           // World ocean surface level

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Config")
    float MaxOxygen = 100.0f;             // Max breath capacity (seconds)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Config")
    float OxygenDepletionRate = 5.0f;     // Breath lost per second underwater (20s breath)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Config")
    float OxygenRecoveryRate = 35.0f;     // Breath restored per second on surface

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Config")
    float DrowningDamagePerSecond = 15.0f;// Damage taken when breath runs out

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Config")
    float SwimForwardSpeed = 450.0f;      // Surface swim speed (cm/s)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Config")
    float DiveVerticalSpeed = 300.0f;     // Ascend/descend speed underwater

    // ─── SFX / VFX ───────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Audio")
    TObjectPtr<USoundBase> WaterEnterSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Audio")
    TObjectPtr<USoundBase> GaspForAirSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Audio")
    TObjectPtr<USoundBase> UnderWaterHeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|VFX")
    TObjectPtr<UNiagaraSystem> WaterSplashFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|VFX")
    TObjectPtr<UNiagaraSystem> BubbleTrailFX;

    // ─── Controls / Queries ──────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Swimming|Actions")
    void SetDiveInput(float Value); // +1 = Surface/Up, -1 = Dive/Down

    UFUNCTION(BlueprintPure, Category = "Swimming|State")
    bool IsInWater() const { return bInWater; }

    UFUNCTION(BlueprintPure, Category = "Swimming|State")
    bool IsUnderwater() const { return bUnderwater; }

    UFUNCTION(BlueprintPure, Category = "Swimming|State")
    float GetOxygenPercent() const { return CurrentOxygen / MaxOxygen; }

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Swimming|Events")
    FOnSubmergedStateChanged OnSubmergedStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Swimming|Events")
    FOnOxygenChanged OnOxygenChanged;

private:
    float CurrentOxygen = 100.0f;
    float DiveInput = 0.0f;
    bool bInWater = false;
    bool bUnderwater = false;

    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<URealWorldHealthComponent> HealthComponent;

    void CheckWaterImmersion(float DeltaTime);
    void UpdateOxygen(float DeltaTime);
};
