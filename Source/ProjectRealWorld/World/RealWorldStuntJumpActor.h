// ============================================================================
// RealWorldStuntJumpActor.h
// Project RealWorld — GTA V-Class Unique Stunt Jumps System
// Features: Ramp trigger, cinematic slow-mo camera tracking,
//           airborne vehicle physics, 4-wheel landing detection, cash & RP bonus
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "RealWorldStuntJumpActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class ARealWorldVehicleBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStuntJumpFinished, bool, bSuccess, int32, CashReward);

UCLASS(Blueprintable)
class PROJECTREALWORLD_API ARealWorldStuntJumpActor : public AActor
{
    GENERATED_BODY()

public:
    ARealWorldStuntJumpActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Components ──────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StuntJump|Components")
    TObjectPtr<UBoxComponent> LaunchTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StuntJump|Components")
    TObjectPtr<UBoxComponent> LandingZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StuntJump|Components")
    TObjectPtr<UStaticMeshComponent> RampMesh;

    // ─── Config ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StuntJump|Config")
    float MinimumLaunchSpeedKmh = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StuntJump|Config")
    int32 CashReward = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StuntJump|Config")
    int32 ReputationReward = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StuntJump|Audio")
    TObjectPtr<USoundBase> JumpSuccessSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StuntJump|Audio")
    TObjectPtr<USoundBase> JumpFailSound;

    UPROPERTY(BlueprintAssignable, Category = "StuntJump|Events")
    FOnStuntJumpFinished OnStuntJumpFinished;

private:
    bool bJumpActive = false;
    float AirborneTimer = 0.0f;

    UPROPERTY()
    TObjectPtr<ARealWorldVehicleBase> ActiveJumpingVehicle;

    UFUNCTION()
    void OnLaunchOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                         bool bFromSweep, const FHitResult& SweepResult);

    void EndStuntJump(bool bSuccessful);
};
