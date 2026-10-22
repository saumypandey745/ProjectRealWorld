// ============================================================================
// RealWorldSuperyachtActor.h
// Project RealWorld — GTA Online Galaxy Superyacht Simulation
// Features: Offshore luxury mansion, helipad landing beacon,
//           watercraft tender dock, automated anti-air missile defenses
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "RealWorldSuperyachtActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UPointLightComponent;
class ARealWorldHelicopterBase;
class ARealWorldBoatBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSuperyachtDefensesToggled, bool, bActive);

UCLASS(Blueprintable)
class PROJECTREALWORLD_API ARealWorldSuperyachtActor : public AActor
{
    GENERATED_BODY()

public:
    ARealWorldSuperyachtActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Visual Components ───────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Superyacht|Components")
    TObjectPtr<UStaticMeshComponent> YachtHullMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Superyacht|Components")
    TObjectPtr<UStaticMeshComponent> HelipadMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Superyacht|Components")
    TObjectPtr<USphereComponent> DefenseZoneSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Superyacht|Components")
    TObjectPtr<UPointLightComponent> HelipadBeaconLight;

    // ─── Config ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Superyacht|Config")
    FString YachtName = TEXT("The Pisces Luxury Superyacht");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Superyacht|Defenses")
    bool bAirDefensesActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Superyacht|Defenses")
    float AirDefenseRadius = 15000.0f; // 150 meters perimeter

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Superyacht|Defenses")
    float DefenseFireInterval = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Superyacht|Audio")
    TObjectPtr<USoundBase> AirDefenseWarningSiren;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Superyacht|Audio")
    TObjectPtr<USoundBase> MissileLaunchSound;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Superyacht|Actions")
    void SetAirDefenses(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Superyacht|State")
    bool AreAirDefensesActive() const { return bAirDefensesActive; }

    UPROPERTY(BlueprintAssignable, Category = "Superyacht|Events")
    FOnSuperyachtDefensesToggled OnSuperyachtDefensesToggled;

private:
    float DefenseCooldownTimer = 0.0f;
    float HelipadPulseTimer = 0.0f;

    void ScanAirDefensePerimeter(float DeltaTime);
    void FireDefenseMissile(AActor* HostileAircraft);
};
