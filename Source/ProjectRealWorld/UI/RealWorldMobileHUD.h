// ============================================================================
// RealWorldMobileHUD.h
// Project RealWorld — GTA5-class Mobile Touch HUD
// Virtual joystick, gesture combat, swipe-to-aim, tap-to-shoot,
// weapon wheel, minimap, health/armor bars, wanted stars
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "RealWorldMobileHUD.generated.h"

class URealWorldCombatComponent;
class UImage;
class UCanvasPanel;
class UProgressBar;
class UTextBlock;
class UHorizontalBox;
class UOverlay;

// ─── Joystick State ───────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FVirtualJoystickState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector2D Center = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FVector2D CurrentInput = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    bool bActive = false;

    int32 TouchIndex = -1;
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTREALWORLD_API URealWorldMobileHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ─── Touch Handling ──────────────────────────────────────────────────────
    virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry,
                                         const FPointerEvent& InGestureEvent) override;
    virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry,
                                       const FPointerEvent& InGestureEvent) override;
    virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry,
                                       const FPointerEvent& InGestureEvent) override;

    // ─── HUD Update Interface ────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "MobileHUD")
    void UpdateHealthBar(float CurrentHealth, float MaxHealth);

    UFUNCTION(BlueprintCallable, Category = "MobileHUD")
    void UpdateArmorBar(float CurrentArmor, float MaxArmor);

    UFUNCTION(BlueprintCallable, Category = "MobileHUD")
    void UpdateAmmoDisplay(int32 CurrentAmmo, int32 ReserveAmmo);

    UFUNCTION(BlueprintCallable, Category = "MobileHUD")
    void UpdateWantedStars(int32 Stars);

    UFUNCTION(BlueprintCallable, Category = "MobileHUD")
    void UpdateMinimapRotation(float HeadingDegrees);

    UFUNCTION(BlueprintCallable, Category = "MobileHUD")
    void ShowWeaponWheel(bool bShow);

    // ─── Joystick Output ─────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "MobileHUD|Input")
    FVector2D MovementJoystickInput = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "MobileHUD|Input")
    FVector2D CameraJoystickInput = FVector2D::ZeroVector;

    // ─── Widget References ───────────────────────────────────────────────────
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCanvasPanel> RootCanvas;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> ArmorBar;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> AmmoText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ReserveAmmoText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UHorizontalBox> WantedStarsBox;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> MinimapImage;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UOverlay> WeaponWheelOverlay;

    // ─── Config ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MobileHUD|Joystick")
    float JoystickRadius = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MobileHUD|Joystick")
    float JoystickDeadzone = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MobileHUD|Joystick")
    float LeftJoystickMaxX = 0.35f; // Normalized screen X boundary for left joystick area

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MobileHUD|Tap")
    float DoubleTapInterval = 0.25f; // Seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MobileHUD|Tap")
    float HoldFireThreshold = 0.15f; // Seconds to distinguish tap from hold-fire

protected:
    // Joystick states
    FVirtualJoystickState LeftJoystick;   // Movement
    FVirtualJoystickState RightJoystick;  // Camera

    // Tap/gesture detection
    float LastTapTime = 0.0f;
    float FireHoldTimer = 0.0f;
    bool bIsHoldingFire = false;

    // Helpers
    FVector2D GetNormalizedJoystickInput(const FVirtualJoystickState& Joystick) const;
    bool IsLeftSide(const FVector2D& ScreenPos, const FGeometry& Geometry) const;
    void ProcessLeftJoystickInput();
    void ProcessRightJoystickInput();
    void OnDoubleTap(const FVector2D& TapPosition);
    void OnSingleTap(const FVector2D& TapPosition);
    void OnSwipeGesture(const FVector2D& SwipeDir);

    // Cached player ref
    UPROPERTY()
    TObjectPtr<APawn> OwnerPawn;

    UPROPERTY()
    TObjectPtr<URealWorldCombatComponent> CombatComp;

    void CachePlayerRefs();
};
