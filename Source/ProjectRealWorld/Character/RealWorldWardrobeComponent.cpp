// ============================================================================
// RealWorldWardrobeComponent.cpp
// Project RealWorld — GTA V-Class Outfits, Wardrobe & Mask Disguise System
// ============================================================================

#include "Character/RealWorldWardrobeComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

URealWorldWardrobeComponent::URealWorldWardrobeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URealWorldWardrobeComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void URealWorldWardrobeComponent::SetOutfit(EOutfitCategory NewOutfit)
{
    if (CurrentOutfit == NewOutfit) return;
    CurrentOutfit = NewOutfit;

    if (ClothesRustleSound && OwnerCharacter)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ClothesRustleSound, OwnerCharacter->GetActorLocation());
    }

    OnOutfitChanged.Broadcast(CurrentOutfit);
}

void URealWorldWardrobeComponent::SetMask(EFaceMaskType NewMask)
{
    if (CurrentMask == NewMask) return;
    CurrentMask = NewMask;

    if (MaskEquipSound && OwnerCharacter)
    {
        UGameplayStatics::PlaySoundAtLocation(this, MaskEquipSound, OwnerCharacter->GetActorLocation());
    }

    OnMaskToggled.Broadcast(CurrentMask);
}

void URealWorldWardrobeComponent::RemoveMask()
{
    SetMask(EFaceMaskType::None);
}

float URealWorldWardrobeComponent::GetPoliceRecognitionMultiplier() const
{
    // Wearing a mask cuts police recognition rate in half (0.5x), delaying wanted stars!
    if (CurrentMask != EFaceMaskType::None)
    {
        return 0.5f;
    }
    return 1.0f;
}
