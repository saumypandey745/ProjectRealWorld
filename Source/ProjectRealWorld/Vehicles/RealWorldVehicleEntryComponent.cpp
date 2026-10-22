// ============================================================================
// RealWorldVehicleEntryComponent.cpp
// Project RealWorld — GTA5-style Vehicle Entry/Exit
// ============================================================================

#include "Vehicles/RealWorldVehicleEntryComponent.h"
#include "Vehicles/RealWorldVehicleBase.h"
#include "Character/RealWorldCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

URealWorldVehicleEntryComponent::URealWorldVehicleEntryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void URealWorldVehicleEntryComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ARealWorldCharacter>(GetOwner());
}

void URealWorldVehicleEntryComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                     FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    switch (EntryState)
    {
        case EVehicleEntryState::None:
        case EVehicleEntryState::Seated:
            TickProximityDetection();
            break;

        case EVehicleEntryState::EnteringAnim:
            TickEntryAnimation(DeltaTime);
            break;

        case EVehicleEntryState::ExitingAnim:
            TickExitAnimation(DeltaTime);
            break;

        default: break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PROXIMITY DETECTION
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldVehicleEntryComponent::TickProximityDetection()
{
    if (!OwnerCharacter) return;

    ARealWorldVehicleBase* Found = FindNearestVehicle();

    if (Found != NearbyVehicle)
    {
        NearbyVehicle = Found;
        // Could broadcast UI hint here (e.g. "Press F to enter vehicle")
    }
}

ARealWorldVehicleBase* URealWorldVehicleEntryComponent::FindNearestVehicle() const
{
    if (!OwnerCharacter) return nullptr;

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    const bool bFound = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        OwnerCharacter->GetActorLocation(),
        FQuat::Identity,
        ECC_Vehicle,
        FCollisionShape::MakeSphere(EntryDetectionRadius),
        QueryParams
    );

    if (!bFound) return nullptr;

    ARealWorldVehicleBase* Nearest    = nullptr;
    float                  NearestDist = FLT_MAX;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        ARealWorldVehicleBase* Vehicle = Cast<ARealWorldVehicleBase>(Overlap.GetActor());
        if (!Vehicle) continue;

        const float Dist = FVector::Dist(OwnerCharacter->GetActorLocation(), Vehicle->GetActorLocation());
        if (Dist < NearestDist)
        {
            NearestDist = Dist;
            Nearest     = Vehicle;
        }
    }

    return Nearest;
}

// ─────────────────────────────────────────────────────────────────────────────
// ENTRY
// ─────────────────────────────────────────────────────────────────────────────
bool URealWorldVehicleEntryComponent::CanEnterVehicle() const
{
    return NearbyVehicle != nullptr
        && EntryState == EVehicleEntryState::None
        && !IsInVehicle();
}

bool URealWorldVehicleEntryComponent::TryEnterNearbyVehicle(EVehicleEntrySeat PreferredSeat)
{
    if (!CanEnterVehicle()) return false;

    CurrentSeat = PreferredSeat;
    BeginEntryAnimation(NearbyVehicle, PreferredSeat);
    return true;
}

void URealWorldVehicleEntryComponent::BeginEntryAnimation(ARealWorldVehicleBase* Vehicle,
                                                           EVehicleEntrySeat Seat)
{
    if (!Vehicle || !OwnerCharacter) return;

    EntryState    = EVehicleEntryState::EnteringAnim;
    EntryAnimTimer = 0.0f;
    OccupiedVehicle = Vehicle;

    // Disable character movement during entry animation
    if (UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement())
    {
        MovComp->DisableMovement();
    }

    // Play door open sound
    if (DoorOpenSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DoorOpenSound, Vehicle->GetActorLocation());
    }

    // Open the door (physics-based door if available)
    Vehicle->OpenDoor(Seat == EVehicleEntrySeat::Driver ? 0 : 1);
}

void URealWorldVehicleEntryComponent::TickEntryAnimation(float DeltaTime)
{
    if (!OwnerCharacter || !OccupiedVehicle) return;

    EntryAnimTimer += DeltaTime;
    const float Alpha = FMath::Clamp(EntryAnimTimer / EntryAnimationDuration, 0.0f, 1.0f);

    // Smoothly move character toward seat socket
    const FName SeatSocket  = GetSeatSocketName(CurrentSeat);
    const FTransform SocketT = OccupiedVehicle->GetMesh()->GetSocketTransform(SeatSocket, RTS_World);

    const FVector TargetLoc  = SocketT.GetLocation();
    const FVector CurrentLoc = OwnerCharacter->GetActorLocation();
    const FVector InterpLoc  = FMath::Lerp(CurrentLoc, TargetLoc, Alpha);

    OwnerCharacter->SetActorLocation(InterpLoc, false, nullptr, ETeleportType::None);
    OwnerCharacter->SetActorRotation(SocketT.GetRotation().Rotator());

    if (Alpha >= 1.0f)
    {
        CompleteEntry();
    }
}

void URealWorldVehicleEntryComponent::CompleteEntry()
{
    if (!OwnerCharacter || !OccupiedVehicle) return;

    EntryState = EVehicleEntryState::Seated;

    // Attach character to vehicle seat
    const FName SeatSocket = GetSeatSocketName(CurrentSeat);
    OwnerCharacter->AttachToComponent(
        OccupiedVehicle->GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        SeatSocket
    );

    // Hide character body (first-person view in vehicle)
    OwnerCharacter->GetMesh()->SetOwnerNoSee(true);

    // Transfer control to vehicle
    if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
    {
        if (CurrentSeat == EVehicleEntrySeat::Driver)
        {
            PC->Possess(OccupiedVehicle);
        }
    }

    // Close door
    if (DoorCloseSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DoorCloseSound, OccupiedVehicle->GetActorLocation());
    }
    OccupiedVehicle->CloseDoor(CurrentSeat == EVehicleEntrySeat::Driver ? 0 : 1);

    OnVehicleEntered.Broadcast(OccupiedVehicle, CurrentSeat);
}

// ─────────────────────────────────────────────────────────────────────────────
// EXIT
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldVehicleEntryComponent::ExitCurrentVehicle(bool bRollOut)
{
    if (!IsInVehicle() || EntryState != EVehicleEntryState::Seated) return;

    BeginExitAnimation(bRollOut);
}

void URealWorldVehicleEntryComponent::BeginExitAnimation(bool bRollOut)
{
    if (!OwnerCharacter || !OccupiedVehicle) return;

    EntryState     = EVehicleEntryState::ExitingAnim;
    EntryAnimTimer = 0.0f;

    // Return control to character
    if (APlayerController* PC = Cast<APlayerController>(OccupiedVehicle->GetController()))
    {
        PC->Possess(OwnerCharacter);
    }

    // Detach from vehicle
    OwnerCharacter->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    OwnerCharacter->GetMesh()->SetOwnerNoSee(false);

    // Compute exit location (side of vehicle, away from traffic)
    const FVector VehicleRight = OccupiedVehicle->GetActorRightVector();
    const bool bExitRight      = (CurrentSeat == EVehicleEntrySeat::Driver);
    ExitLocation = OccupiedVehicle->GetActorLocation() + VehicleRight * (bExitRight ? -200.0f : 200.0f);
    ExitLocation.Z = OwnerCharacter->GetActorLocation().Z;

    // Play door open sound
    if (DoorOpenSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DoorOpenSound, OccupiedVehicle->GetActorLocation());
    }
    OccupiedVehicle->OpenDoor(bExitRight ? 0 : 1);
}

void URealWorldVehicleEntryComponent::TickExitAnimation(float DeltaTime)
{
    if (!OwnerCharacter) return;

    EntryAnimTimer += DeltaTime;
    const float Alpha = FMath::Clamp(EntryAnimTimer / ExitAnimationDuration, 0.0f, 1.0f);

    const FVector CurrentLoc = OwnerCharacter->GetActorLocation();
    OwnerCharacter->SetActorLocation(FMath::Lerp(CurrentLoc, ExitLocation, Alpha),
                                      false, nullptr, ETeleportType::None);

    if (Alpha >= 1.0f)
    {
        CompleteExit();
    }
}

void URealWorldVehicleEntryComponent::CompleteExit()
{
    if (!OwnerCharacter) return;

    ARealWorldVehicleBase* ExitedVehicle = OccupiedVehicle;

    // Re-enable character movement
    if (UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement())
    {
        MovComp->SetMovementMode(EMovementMode::MOVE_Walking);
    }

    // Close door
    if (DoorCloseSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DoorCloseSound,
                                              OccupiedVehicle->GetActorLocation());
    }
    OccupiedVehicle->CloseDoor(CurrentSeat == EVehicleEntrySeat::Driver ? 0 : 1);

    OccupiedVehicle = nullptr;
    EntryState       = EVehicleEntryState::None;

    OnVehicleExited.Broadcast(ExitedVehicle);
}

// ─────────────────────────────────────────────────────────────────────────────
// HELPERS
// ─────────────────────────────────────────────────────────────────────────────
FName URealWorldVehicleEntryComponent::GetSeatSocketName(EVehicleEntrySeat Seat) const
{
    switch (Seat)
    {
        case EVehicleEntrySeat::Driver:     return TEXT("Seat_Driver");
        case EVehicleEntrySeat::Passenger:  return TEXT("Seat_Passenger");
        case EVehicleEntrySeat::RearLeft:   return TEXT("Seat_RearLeft");
        case EVehicleEntrySeat::RearRight:  return TEXT("Seat_RearRight");
        default:                            return TEXT("Seat_Driver");
    }
}
