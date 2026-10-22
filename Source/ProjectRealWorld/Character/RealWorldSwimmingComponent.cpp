// ============================================================================
// RealWorldSwimmingComponent.cpp
// Project RealWorld — GTA V-Class Swimming & Deep-Water Diving System
// ============================================================================

#include "Character/RealWorldSwimmingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RealWorldHealthComponent.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"

URealWorldSwimmingComponent::URealWorldSwimmingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void URealWorldSwimmingComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        HealthComponent = OwnerCharacter->FindComponentByClass<URealWorldHealthComponent>();
    }
    CurrentOxygen = MaxOxygen;
}

void URealWorldSwimmingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter) return;

    CheckWaterImmersion(DeltaTime);
    UpdateOxygen(DeltaTime);
}

void URealWorldSwimmingComponent::CheckWaterImmersion(float DeltaTime)
{
    const FVector HeadLoc = OwnerCharacter->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
    const FVector FeetLoc = OwnerCharacter->GetActorLocation() - FVector(0.0f, 0.0f, 80.0f);

    const bool bPrevInWater = bInWater;
    const bool bPrevUnderwater = bUnderwater;

    bInWater = (FeetLoc.Z < WaterSurfaceZ);
    bUnderwater = (HeadLoc.Z < WaterSurfaceZ);

    UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();

    // Transitioning into water
    if (bInWater && !bPrevInWater)
    {
        if (WaterEnterSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, WaterEnterSound, OwnerCharacter->GetActorLocation());
        }
        if (MoveComp)
        {
            MoveComp->SetMovementMode(MOVE_Swimming);
            MoveComp->MaxSwimSpeed = SwimForwardSpeed;
        }
    }
    // Transitioning out of water
    else if (!bInWater && bPrevInWater)
    {
        if (MoveComp && MoveComp->MovementMode == MOVE_Swimming)
        {
            MoveComp->SetMovementMode(MOVE_Walking);
        }
    }

    // Underwater vertical control
    if (bUnderwater && MoveComp)
    {
        if (FMath::Abs(DiveInput) > 0.1f)
        {
            MoveComp->Velocity.Z = DiveInput * DiveVerticalSpeed;
        }
    }

    // Surfacing (gasp for air)
    if (!bUnderwater && bPrevUnderwater)
    {
        if (GaspForAirSound && CurrentOxygen < (MaxOxygen * 0.5f))
        {
            UGameplayStatics::PlaySoundAtLocation(this, GaspForAirSound, HeadLoc);
        }
        OnSubmergedStateChanged.Broadcast(false);
    }
    else if (bUnderwater && !bPrevUnderwater)
    {
        OnSubmergedStateChanged.Broadcast(true);
    }
}

void URealWorldSwimmingComponent::UpdateOxygen(float DeltaTime)
{
    if (bUnderwater)
    {
        // Deplete breath
        CurrentOxygen = FMath::Max(CurrentOxygen - (OxygenDepletionRate * DeltaTime), 0.0f);

        // Drowning damage if out of oxygen
        if (CurrentOxygen <= 0.0f && HealthComponent)
        {
            FDamageEvent DmgEvent;
            HealthComponent->TakeDamage(DrowningDamagePerSecond * DeltaTime, DmgEvent, nullptr, OwnerCharacter);
        }
    }
    else
    {
        // Recover breath rapidly on surface
        CurrentOxygen = FMath::Min(CurrentOxygen + (OxygenRecoveryRate * DeltaTime), MaxOxygen);
    }

    OnOxygenChanged.Broadcast(CurrentOxygen, MaxOxygen);
}

void URealWorldSwimmingComponent::SetDiveInput(float Value)
{
    DiveInput = FMath::Clamp(Value, -1.0f, 1.0f);
}
