// ============================================================================
// RealWorldSmartphoneWidget.cpp
// Project RealWorld — GTA5-class In-Game Smartphone (iFruit Style) Implementation
// ============================================================================

#include "UI/RealWorldSmartphoneWidget.h"
#include "Core/RealWorldSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RealWorldCharacter.h"
#include "Subsystems/MissionSubsystem.h"
#include "Subsystems/TimeOfDaySubsystem.h"

void URealWorldSmartphoneWidget::NativeConstruct()
{
    Super::NativeConstruct();

    PopulateDefaultContacts();
    bIsOpen = false;
    CurrentSlideProgress = 0.0f;
    SetRenderTranslation(FVector2D(0.0f, 600.0f)); // Start hidden offscreen below
    SetVisibility(ESlateVisibility::Hidden);
}

void URealWorldSmartphoneWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Smooth slide-up / slide-down animation (GTA5 style)
    const float TargetProgress = bIsOpen ? 1.0f : 0.0f;
    CurrentSlideProgress = FMath::FInterpTo(CurrentSlideProgress, TargetProgress, InDeltaTime, SlideSpeed);

    // Translation: 0.0f is fully open, 600.0f is hidden offscreen bottom
    const float YOffset = FMath::Lerp(600.0f, 0.0f, CurrentSlideProgress);
    SetRenderTranslation(FVector2D(0.0f, YOffset));

    if (!bIsOpen && CurrentSlideProgress <= 0.01f)
    {
        SetVisibility(ESlateVisibility::Hidden);
    }

    // Call duration timer
    if (CurrentScreen == EPhoneScreen::CallActive)
    {
        ActiveCallTimer += InDeltaTime;
    }
}

void URealWorldSmartphoneWidget::TogglePhone()
{
    if (bIsOpen)
    {
        ClosePhone();
    }
    else
    {
        OpenPhone();
    }
}

void URealWorldSmartphoneWidget::OpenPhone()
{
    bIsOpen = true;
    SetVisibility(ESlateVisibility::Visible);
    PlayPhoneSound(PhoneOpenSound);
    CurrentScreen = EPhoneScreen::Home;
}

void URealWorldSmartphoneWidget::ClosePhone()
{
    bIsOpen = false;
    PlayPhoneSound(PhoneClickSound);
    if (CurrentScreen == EPhoneScreen::CallActive)
    {
        EndCall();
    }
}

void URealWorldSmartphoneWidget::NavigateToScreen(EPhoneScreen NewScreen)
{
    CurrentScreen = NewScreen;
    PlayPhoneSound(PhoneClickSound);

    if (NewScreen == EPhoneScreen::Bank)
    {
        RefreshBankBalance();
    }
}

void URealWorldSmartphoneWidget::PressHomeButton()
{
    if (CurrentScreen == EPhoneScreen::Home)
    {
        ClosePhone();
    }
    else
    {
        NavigateToScreen(EPhoneScreen::Home);
    }
}

void URealWorldSmartphoneWidget::InitiateCall(FName ContactID)
{
    ActiveCallContactID = ContactID;
    ActiveCallTimer = 0.0f;
    CurrentScreen = EPhoneScreen::CallActive;

    PlayPhoneSound(PhoneRingtoneSound);
    OnPhoneCallStarted.Broadcast(ContactID);
}

void URealWorldSmartphoneWidget::EndCall()
{
    ActiveCallContactID = NAME_None;
    ActiveCallTimer = 0.0f;
    CurrentScreen = EPhoneScreen::Contacts;

    PlayPhoneSound(PhoneClickSound);
    OnPhoneCallEnded.Broadcast();
}

void URealWorldSmartphoneWidget::ExecuteQuickSave()
{
    UWorld* World = GetWorld();
    if (!World) return;

    ARealWorldCharacter* PlayerChar = Cast<ARealWorldCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    if (!PlayerChar) return;

    URealWorldSaveGame* SaveObj = Cast<URealWorldSaveGame>(
        UGameplayStatics::CreateSaveGameObject(URealWorldSaveGame::StaticClass())
    );

    if (SaveObj)
    {
        SaveObj->PlayerTransform = PlayerChar->GetActorTransform();
        SaveObj->Timestamp = FDateTime::Now();

        const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveObj, TEXT("QuickSave"), 0);
        PlayPhoneSound(PhoneClickSound);
        OnPhoneQuickSaved.Broadcast(bSaved);
    }
}

void URealWorldSmartphoneWidget::SetGPSWaypointToActiveObjective()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (UMissionSubsystem* MissionSub = World->GetSubsystem<UMissionSubsystem>())
    {
        PlayPhoneSound(PhoneClickSound);
    }
}

void URealWorldSmartphoneWidget::RefreshBankBalance()
{
    PlayPhoneSound(PhoneClickSound);
}

void URealWorldSmartphoneWidget::TakeSelfieSnapshot()
{
    PlayPhoneSound(CameraShutterSound);
}

void URealWorldSmartphoneWidget::UpdateStatusBar(const FString& InTime, int32 InBatteryPercent, int32 InSignalBars)
{
}

void URealWorldSmartphoneWidget::PopulateDefaultContacts()
{
    ContactsList.Empty();

    {
        FPhoneContact Contact;
        Contact.ContactID = TEXT("Malik_Fixer");
        Contact.DisplayName = FText::FromString(TEXT("Malik (Harbor Fixer)"));
        Contact.RoleDescription = FText::FromString(TEXT("Courier & Transport Missions"));
        Contact.bHasAvailableMission = true;
        ContactsList.Add(Contact);
    }
    {
        FPhoneContact Contact;
        Contact.ContactID = TEXT("Tanya_Broker");
        Contact.DisplayName = FText::FromString(TEXT("Tanya (Crown Street)"));
        Contact.RoleDescription = FText::FromString(TEXT("Vehicle Import / Export"));
        Contact.bHasAvailableMission = true;
        ContactsList.Add(Contact);
    }
    {
        FPhoneContact Contact;
        Contact.ContactID = TEXT("Emergency_911");
        Contact.DisplayName = FText::FromString(TEXT("911 Dispatch"));
        Contact.RoleDescription = FText::FromString(TEXT("Emergency Services"));
        Contact.bHasAvailableMission = false;
        ContactsList.Add(Contact);
    }
}

void URealWorldSmartphoneWidget::PlayPhoneSound(USoundBase* Sound)
{
    if (Sound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Sound);
    }
}
