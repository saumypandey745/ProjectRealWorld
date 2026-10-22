// ============================================================================
// RealWorldDialogueSubsystem.h
// Project RealWorld — GTA5-class Cinematic Cutscene & Dialogue Pipeline
// Cinematic 2.39:1 letterbox bars, Hindi & English subtitles, dynamic camera cuts
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Sound/SoundBase.h"
#include "RealWorldDialogueSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FDialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText EnglishText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText HindiText; // हिंदी डायलॉग

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<USoundBase> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform CameraShotTransform = FTransform::Identity;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSubtitleDisplayed, const FText&, Speaker, const FText&, SubtitleText, bool, bIsHindi);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLetterboxStateChanged, bool, bActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueSequenceFinished);

UCLASS()
class PROJECTREALWORLD_API URealWorldDialogueSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

    // ─── Playback ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Cinematics|Dialogue")
    void PlayDialogueSequence(const TArray<FDialogueLine>& InLines);

    UFUNCTION(BlueprintCallable, Category = "Cinematics|Dialogue")
    void SkipCurrentLine();

    UFUNCTION(BlueprintCallable, Category = "Cinematics|Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Cinematics|Dialogue")
    void SetLanguagePreference(bool bHindi);

    UFUNCTION(BlueprintPure, Category = "Cinematics|Dialogue")
    bool IsLetterboxActive() const { return bLetterboxActive; }

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Cinematics|Events")
    FOnSubtitleDisplayed OnSubtitleDisplayed;

    UPROPERTY(BlueprintAssignable, Category = "Cinematics|Events")
    FOnLetterboxStateChanged OnLetterboxStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Cinematics|Events")
    FOnDialogueSequenceFinished OnDialogueSequenceFinished;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematics|State")
    bool bLetterboxActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematics|State")
    bool bUseHindiSubtitles = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematics|State")
    int32 CurrentLineIndex = -1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematics|State")
    float LineTimer = 0.0f;

    TArray<FDialogueLine> ActiveSequence;

    void DisplayLine(int32 Index);
};
