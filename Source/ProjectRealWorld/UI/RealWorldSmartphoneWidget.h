// ============================================================================
// RealWorldSmartphoneWidget.h
// Project RealWorld — GTA5-class In-Game Smartphone (iFruit Style)
// Quick Save, Contacts & Calling, GPS Waypoint, Bank Balance, Camera Selfie
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RealWorldSmartphoneWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;
class UWidgetSwitcher;
class UVerticalBox;

UENUM(BlueprintType)
enum class EPhoneScreen : uint8
{
    Home        UMETA(DisplayName = "Home"),
    Contacts    UMETA(DisplayName = "Contacts"),
    CallActive  UMETA(DisplayName = "Call Active"),
    GPS         UMETA(DisplayName = "GPS Navigation"),
    Bank        UMETA(DisplayName = "Maze Bank"),
    Settings    UMETA(DisplayName = "Settings"),
    Camera      UMETA(DisplayName = "Camera")
};

USTRUCT(BlueprintType)
struct FPhoneContact
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ContactID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText RoleDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasAvailableMission = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhoneCallStarted, FName, ContactID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhoneCallEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhoneQuickSaved, bool, bSuccess);

UCLASS(BlueprintType, Blueprintable)
class PROJECTREALWORLD_API URealWorldSmartphoneWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ─── Visibility & State ──────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Smartphone")
    void TogglePhone();

    UFUNCTION(BlueprintCallable, Category = "Smartphone")
    void OpenPhone();

    UFUNCTION(BlueprintCallable, Category = "Smartphone")
    void ClosePhone();

    UFUNCTION(BlueprintPure, Category = "Smartphone")
    bool IsPhoneOpen() const { return bIsOpen; }

    // ─── Navigation ──────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Smartphone")
    void NavigateToScreen(EPhoneScreen NewScreen);

    UFUNCTION(BlueprintCallable, Category = "Smartphone")
    void PressHomeButton();

    // ─── App Functions ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Smartphone|Contacts")
    void InitiateCall(FName ContactID);

    UFUNCTION(BlueprintCallable, Category = "Smartphone|Contacts")
    void EndCall();

    UFUNCTION(BlueprintCallable, Category = "Smartphone|Save")
    void ExecuteQuickSave();

    UFUNCTION(BlueprintCallable, Category = "Smartphone|GPS")
    void SetGPSWaypointToActiveObjective();

    UFUNCTION(BlueprintCallable, Category = "Smartphone|Bank")
    void RefreshBankBalance();

    UFUNCTION(BlueprintCallable, Category = "Smartphone|Camera")
    void TakeSelfieSnapshot();

    // ─── Status Bar ──────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Smartphone")
    void UpdateStatusBar(const FString& InTime, int32 InBatteryPercent, int32 InSignalBars);

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Smartphone|Events")
    FOnPhoneCallStarted OnPhoneCallStarted;

    UPROPERTY(BlueprintAssignable, Category = "Smartphone|Events")
    FOnPhoneCallEnded OnPhoneCallEnded;

    UPROPERTY(BlueprintAssignable, Category = "Smartphone|Events")
    FOnPhoneQuickSaved OnPhoneQuickSaved;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Smartphone|State")
    bool bIsOpen = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Smartphone|State")
    EPhoneScreen CurrentScreen = EPhoneScreen::Home;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smartphone|Config")
    TArray<FPhoneContact> ContactsList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smartphone|Audio")
    TObjectPtr<USoundBase> PhoneOpenSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smartphone|Audio")
    TObjectPtr<USoundBase> PhoneClickSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smartphone|Audio")
    TObjectPtr<USoundBase> PhoneRingtoneSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smartphone|Audio")
    TObjectPtr<USoundBase> CameraShutterSound;

    // Slide-up animation translation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smartphone|Animation")
    float SlideSpeed = 12.0f;

    float CurrentSlideProgress = 0.0f; // 0 = hidden offscreen bottom, 1 = fully visible

    // Active call state
    FName ActiveCallContactID = NAME_None;
    float ActiveCallTimer = 0.0f;

    void PopulateDefaultContacts();
    void PlayPhoneSound(USoundBase* Sound);
};
