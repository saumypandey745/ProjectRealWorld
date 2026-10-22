// ============================================================================
// RealWorldAudioBarkComponent.cpp
// Project RealWorld — GTA5-class NPC Contextual Voice Bark System
// ============================================================================

#include "Audio/RealWorldAudioBarkComponent.h"
#include "Kismet/GameplayStatics.h"

URealWorldAudioBarkComponent::URealWorldAudioBarkComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    BarkCooldown = 4.0f;
    LastBarkTimestamp = -100.0f;
}

bool URealWorldAudioBarkComponent::PlayBark(EBarkTrigger Trigger)
{
    const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if ((CurrentTime - LastBarkTimestamp) < BarkCooldown)
    {
        return false; // In cooldown
    }

    LastBarkTimestamp = CurrentTime;

    switch (Trigger)
    {
        case EBarkTrigger::HonkedAt:
            PlayRandomSoundFromList(HonkReactions);
            break;

        case EBarkTrigger::GunPointed:
        case EBarkTrigger::CivSurrender:
            PlayRandomSoundFromList(GunPointedReactions);
            break;

        case EBarkTrigger::NearMissVehicle:
        case EBarkTrigger::PhysicalBump:
            PlayRandomSoundFromList(NearMissReactions);
            break;

        case EBarkTrigger::PolicePursuitHalt:
        case EBarkTrigger::PoliceLethalForce:
            PlayRandomSoundFromList(PoliceOrders);
            break;

        default:
            break;
    }

    return true;
}

void URealWorldAudioBarkComponent::PlayRandomSoundFromList(const TArray<TObjectPtr<USoundBase>>& SoundList)
{
    if (SoundList.Num() == 0 || !GetOwner() || !GetWorld()) return;

    const int32 RandomIndex = FMath::RandRange(0, SoundList.Num() - 1);
    if (USoundBase* Sound = SoundList[RandomIndex])
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(), Sound, GetOwner()->GetActorLocation(),
            1.0f, 1.0f, 0.0f, nullptr, nullptr
        );
    }
}
