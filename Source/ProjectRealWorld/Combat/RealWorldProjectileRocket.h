// ============================================================================
// RealWorldProjectileRocket.h
// Project RealWorld — GTA5-class RPG Rocket Launcher Projectile
// High-velocity flight, smoke particle trail, vehicle flip radial explosion
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealWorldProjectileRocket.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;
class USoundBase;

UCLASS(BlueprintType, Blueprintable)
class PROJECTREALWORLD_API ARealWorldProjectileRocket : public AActor
{
    GENERATED_BODY()

public:
    ARealWorldProjectileRocket();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rocket|Collision")
    TObjectPtr<USphereComponent> CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rocket|Mesh")
    TObjectPtr<UStaticMeshComponent> RocketMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rocket|Movement")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    // ─── Explosion Config ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocket|Damage")
    float BaseExplosionDamage = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocket|Damage")
    float InnerExplosionRadius = 250.0f; // cm (full damage)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocket|Damage")
    float OuterExplosionRadius = 800.0f; // cm (falloff radius)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocket|Damage")
    float ExplosionImpulseForce = 150000.0f; // Flips cars and launches ragdolls

    // ─── VFX / SFX ───────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocket|VFX")
    TObjectPtr<UNiagaraSystem> ExplosionVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocket|Audio")
    TObjectPtr<USoundBase> ExplosionSound;

protected:
    UFUNCTION()
    void OnRocketHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                     UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                     const FHitResult& Hit);

    void Detonate(const FVector& ExplosionLocation);
};
