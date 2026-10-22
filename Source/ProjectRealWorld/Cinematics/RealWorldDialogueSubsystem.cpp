// ============================================================================
// RealWorldDialogueSubsystem.cpp
// Project RealWorld — GTA5-class Cinematic Cutscene & Dialogue Pipeline
// ============================================================================

#include "Cinematics/RealWorldDialogueSubsystem.h"
#include "Kismet/GameplayStatics.h"

void URealWorldDialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    bLetterboxActive = false;
    bUseHindiSubtitles = false;
    CurrentLineIndex = -1;
    LineTimer = 0.0f;
}

void URealWorldDialogueSubsystem::Tick(float DeltaTime)
{
    if (CurrentLineIndex < 0 || !ActiveSequence.IsValidIndex(CurrentLineIndex))
    {
        return;
    }

    LineTimer += DeltaTime;
    const FDialogueLine& CurrentLine = ActiveSequence[CurrentLineIndex];

    if (LineTimer >= CurrentLine.Duration)
    {
        DisplayLine(CurrentLineIndex + 1);
    }
}

TStatId URealWorldDialogueSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URealWorldDialogueSubsystem, STATGROUP_Tickables);
}

void URealWorldDialogueSubsystem::PlayDialogueSequence(const TArray<FDialogueLine>& InLines)
{
    if (InLines.Num() == 0) return;

    ActiveSequence = InLines;
    bLetterboxActive = true;
    OnLetterboxStateChanged.Broadcast(true);

    DisplayLine(0);
}

void URealWorldDialogueSubsystem::SkipCurrentLine()
{
    if (CurrentLineIndex >= 0)
    {
        DisplayLine(CurrentLineIndex + 1);
    }
}

void URealWorldDialogueSubsystem::StopDialogue()
{
    CurrentLineIndex = -1;
    LineTimer = 0.0f;
    ActiveSequence.Empty();
    bLetterboxActive = false;

    OnLetterboxStateChanged.Broadcast(false);
    OnDialogueSequenceFinished.Broadcast();
}

void URealWorldDialogueSubsystem::SetLanguagePreference(bool bHindi)
{
    bUseHindiSubtitles = bHindi;
}

void URealWorldDialogueSubsystem::DisplayLine(int32 Index)
{
    if (!ActiveSequence.IsValidIndex(Index))
    {
        StopDialogue();
        return;
    }

    CurrentLineIndex = Index;
    LineTimer = 0.0f;
    const FDialogueLine& Line = ActiveSequence[Index];

    // Play optional voice clip
    if (Line.VoiceClip && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Line.VoiceClip);
    }

    // Broadcast subtitle according to language preference
    const FText& TextToDisplay = (bUseHindiSubtitles && !Line.HindiText.IsEmpty())
                               ? Line.HindiText
                               : Line.EnglishText;

    OnSubtitleDisplayed.Broadcast(Line.SpeakerName, TextToDisplay, bUseHindiSubtitles);
}
