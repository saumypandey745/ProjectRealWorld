// ============================================================================
// RealWorldAudioBarkComponent.h
// Project RealWorld — GTA5-class NPC Contextual Voice Bark System
// Honking insult, Gun-aim surrender scream, Collision reaction, Police warnings
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "RealWorldAudioBarkComponent.generated.h"

UENUM(BlueprintType)
enum class EBarkTrigger : uint8
{
    HonkedAt            UMETA(DisplayName = "Honked At by Vehicle"),
    GunPointed          UMETA(DisplayName = "Gun Pointed at NPC"),
    NearMissVehicle     UMETA(DisplayName = "Near Miss by Speeding Car"),
    PhysicalBump        UMETA(DisplayName = "Bumped into Pedestrian"),
    PolicePursuitHalt   UMETA(DisplayName = "Police: Halt / Pull Over"),
    PoliceLethalForce   UMETA(DisplayName = "Police: Opening Fire"),
    CivSurrender        UMETA(DisplayName = "Civilian: Don't Shoot!")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREALWORLD_API URealWorldAudioBarkComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URealWorldAudioBarkComponent();

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Barks")
    bool PlayBark(EBarkTrigger Trigger);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Barks")
    float BarkCooldown = 4.0f; // Seconds between dialogue lines to avoid audio spam

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Barks")
    TArray<TObjectPtr<USoundBase>> HonkReactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Barks")
    TArray<TObjectPtr<USoundBase>> GunPointedReactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Barks")
    TArray<TObjectPtr<USoundBase>> NearMissReactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Barks")
    TArray<TObjectPtr<USoundBase>> PoliceOrders;

protected:
    float LastBarkTimestamp = -100.0f;

    void PlayRandomSoundFromList(const TArray<TObjectPtr<USoundBase>>& SoundList);
};
