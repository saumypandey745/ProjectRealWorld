// ============================================================================
// RealWorldMobileHUD.cpp
// Project RealWorld — GTA5-class Mobile Touch HUD Implementation
// ============================================================================

#include "UI/RealWorldMobileHUD.h"
#include "Combat/RealWorldCombatComponent.h"
#include "Character/RealWorldCharacter.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void URealWorldMobileHUD::NativeConstruct()
{
    Super::NativeConstruct();
    CachePlayerRefs();

    // Hide weapon wheel by default
    if (WeaponWheelOverlay)
    {
        WeaponWheelOverlay->SetVisibility(ESlateVisibility::Hidden);
    }
}

void URealWorldMobileHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Process joystick inputs into movement/camera
    if (LeftJoystick.bActive)  ProcessLeftJoystickInput();
    if (RightJoystick.bActive) ProcessRightJoystickInput();

    // Hold-fire detection
    if (bIsHoldingFire)
    {
        FireHoldTimer += InDeltaTime;
        if (FireHoldTimer >= HoldFireThreshold && CombatComp)
        {
            CombatComp->FireCurrentWeapon();
        }
    }
}

void URealWorldMobileHUD::CachePlayerRefs()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        OwnerPawn = PC->GetPawn();
        if (OwnerPawn)
        {
            CombatComp = OwnerPawn->FindComponentByClass<URealWorldCombatComponent>();
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TOUCH INPUT
// ─────────────────────────────────────────────────────────────────────────────
FReply URealWorldMobileHUD::NativeOnTouchStarted(const FGeometry& InGeometry,
                                                   const FPointerEvent& InGestureEvent)
{
    const FVector2D ScreenPos = InGestureEvent.GetScreenSpacePosition();
    const int32 TouchIndex    = (int32)InGestureEvent.GetPointerIndex();

    if (IsLeftSide(ScreenPos, InGeometry))
    {
        // Left side → Movement joystick
        if (!LeftJoystick.bActive)
        {
            LeftJoystick.bActive     = true;
            LeftJoystick.Center      = ScreenPos;
            LeftJoystick.TouchIndex  = TouchIndex;
            LeftJoystick.CurrentInput = FVector2D::ZeroVector;
        }
    }
    else
    {
        // Right side → Camera joystick OR tap actions
        if (!RightJoystick.bActive)
        {
            RightJoystick.bActive     = true;
            RightJoystick.Center      = ScreenPos;
            RightJoystick.TouchIndex  = TouchIndex;
            RightJoystick.CurrentInput = FVector2D::ZeroVector;

            // Start fire hold timer
            bIsHoldingFire = true;
            FireHoldTimer  = 0.0f;
        }
    }

    return FReply::Handled();
}

FReply URealWorldMobileHUD::NativeOnTouchMoved(const FGeometry& InGeometry,
                                                const FPointerEvent& InGestureEvent)
{
    const FVector2D ScreenPos = InGestureEvent.GetScreenSpacePosition();
    const int32 TouchIndex    = (int32)InGestureEvent.GetPointerIndex();

    if (LeftJoystick.bActive && LeftJoystick.TouchIndex == TouchIndex)
    {
        LeftJoystick.CurrentInput = ScreenPos - LeftJoystick.Center;
        // Clamp to radius
        if (LeftJoystick.CurrentInput.Size() > JoystickRadius)
        {
            LeftJoystick.CurrentInput = LeftJoystick.CurrentInput.GetSafeNormal() * JoystickRadius;
        }
    }
    else if (RightJoystick.bActive && RightJoystick.TouchIndex == TouchIndex)
    {
        const FVector2D Delta = ScreenPos - RightJoystick.Center;
        RightJoystick.CurrentInput = Delta;

        // If finger moved significantly, not a tap — cancel fire-hold
        if (Delta.Size() > 20.0f)
        {
            bIsHoldingFire = false;
        }

        // Camera rotation
        if (OwnerPawn)
        {
            if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
            {
                PC->AddYawInput(Delta.X * 0.15f);
                PC->AddPitchInput(-Delta.Y * 0.15f);
            }
        }

        RightJoystick.Center = ScreenPos; // Reset delta center for smooth camera
    }

    return FReply::Handled();
}

FReply URealWorldMobileHUD::NativeOnTouchEnded(const FGeometry& InGeometry,
                                                const FPointerEvent& InGestureEvent)
{
    const int32 TouchIndex = (int32)InGestureEvent.GetPointerIndex();
    const FVector2D ScreenPos = InGestureEvent.GetScreenSpacePosition();

    if (LeftJoystick.bActive && LeftJoystick.TouchIndex == TouchIndex)
    {
        LeftJoystick.bActive      = false;
        LeftJoystick.CurrentInput = FVector2D::ZeroVector;
        MovementJoystickInput     = FVector2D::ZeroVector;
    }
    else if (RightJoystick.bActive && RightJoystick.TouchIndex == TouchIndex)
    {
        RightJoystick.bActive      = false;
        RightJoystick.CurrentInput = FVector2D::ZeroVector;

        // If it was a quick tap (not held for fire)
        if (bIsHoldingFire && FireHoldTimer < HoldFireThreshold)
        {
            // Single tap → check double tap
            const float Now = GetWorld()->GetTimeSeconds();
            if ((Now - LastTapTime) < DoubleTapInterval)
            {
                OnDoubleTap(ScreenPos);
            }
            else
            {
                OnSingleTap(ScreenPos);
            }
            LastTapTime = Now;
        }

        bIsHoldingFire = false;
        FireHoldTimer  = 0.0f;
    }

    return FReply::Handled();
}

// ─────────────────────────────────────────────────────────────────────────────
// GESTURE ACTIONS
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldMobileHUD::OnDoubleTap(const FVector2D& TapPosition)
{
    // Double-tap right side → Melee attack
    if (CombatComp)
    {
        CombatComp->PerformMeleeAttack();
    }
}

void URealWorldMobileHUD::OnSingleTap(const FVector2D& TapPosition)
{
    // Single tap → Fire
    if (CombatComp)
    {
        CombatComp->FireCurrentWeapon();
    }
}

void URealWorldMobileHUD::OnSwipeGesture(const FVector2D& SwipeDir)
{
    if (!CombatComp || !OwnerPawn) return;

    // Swipe left/right → roll/dodge
    if (FMath::Abs(SwipeDir.X) > FMath::Abs(SwipeDir.Y))
    {
        // Horizontal swipe — dodge
        // Broadcast dodge event or call character dodge function
    }
    else if (SwipeDir.Y < -50.0f)
    {
        // Swipe up → enter cover
        CombatComp->TryEnterCover();
    }
    else if (SwipeDir.Y > 50.0f)
    {
        // Swipe down → exit cover / prone
        CombatComp->ExitCover();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// JOYSTICK PROCESSING
// ─────────────────────────────────────────────────────────────────────────────
FVector2D URealWorldMobileHUD::GetNormalizedJoystickInput(const FVirtualJoystickState& Joystick) const
{
    if (!Joystick.bActive) return FVector2D::ZeroVector;

    FVector2D Normalized = Joystick.CurrentInput / JoystickRadius;
    Normalized.X = FMath::Clamp(Normalized.X, -1.0f, 1.0f);
    Normalized.Y = FMath::Clamp(Normalized.Y, -1.0f, 1.0f);

    // Apply deadzone
    if (Normalized.Size() < JoystickDeadzone)
    {
        return FVector2D::ZeroVector;
    }

    return Normalized;
}

void URealWorldMobileHUD::ProcessLeftJoystickInput()
{
    if (!OwnerPawn) return;

    MovementJoystickInput = GetNormalizedJoystickInput(LeftJoystick);

    // Apply to character movement
    OwnerPawn->AddMovementInput(OwnerPawn->GetActorForwardVector(), MovementJoystickInput.Y);
    OwnerPawn->AddMovementInput(OwnerPawn->GetActorRightVector(),   MovementJoystickInput.X);
}

void URealWorldMobileHUD::ProcessRightJoystickInput()
{
    CameraJoystickInput = GetNormalizedJoystickInput(RightJoystick);
    // Camera is handled in touch moved for real-time responsiveness
}

bool URealWorldMobileHUD::IsLeftSide(const FVector2D& ScreenPos, const FGeometry& Geometry) const
{
    const FVector2D LocalPos = Geometry.AbsoluteToLocal(ScreenPos);
    return (LocalPos.X / Geometry.GetLocalSize().X) < LeftJoystickMaxX;
}

// ─────────────────────────────────────────────────────────────────────────────
// HUD UPDATES
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldMobileHUD::UpdateHealthBar(float CurrentHealth, float MaxHealth)
{
    if (HealthBar)
    {
        HealthBar->SetPercent(MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f);

        // Color lerp: green (1) → yellow (0.5) → red (0)
        const float Pct = CurrentHealth / MaxHealth;
        const FLinearColor HealthColor = Pct > 0.5f
            ? FLinearColor::LerpUsingHSV(FLinearColor::Yellow, FLinearColor::Green, (Pct - 0.5f) * 2.0f)
            : FLinearColor::LerpUsingHSV(FLinearColor::Red,    FLinearColor::Yellow, Pct * 2.0f);
        HealthBar->SetFillColorAndOpacity(HealthColor);
    }
}

void URealWorldMobileHUD::UpdateArmorBar(float CurrentArmor, float MaxArmor)
{
    if (ArmorBar)
    {
        ArmorBar->SetPercent(MaxArmor > 0.0f ? CurrentArmor / MaxArmor : 0.0f);
        ArmorBar->SetFillColorAndOpacity(FLinearColor(0.3f, 0.7f, 1.0f)); // Blue
    }
}

void URealWorldMobileHUD::UpdateAmmoDisplay(int32 CurrentAmmo, int32 ReserveAmmo)
{
    if (AmmoText)
    {
        AmmoText->SetText(FText::AsNumber(CurrentAmmo));

        // Red tint when low ammo
        const FSlateColor LowAmmoColor = FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f));
        const FSlateColor NormalColor  = FSlateColor(FLinearColor::White);
        AmmoText->SetColorAndOpacity(CurrentAmmo <= 3 ? LowAmmoColor : NormalColor);
    }

    if (ReserveAmmoText)
    {
        ReserveAmmoText->SetText(FText::Format(FText::FromString(TEXT("/{0}")),
                                                FText::AsNumber(ReserveAmmo)));
    }
}

void URealWorldMobileHUD::UpdateWantedStars(int32 Stars)
{
    if (!WantedStarsBox) return;

    // Iterate through star widgets and set visibility
    const int32 ChildCount = WantedStarsBox->GetChildrenCount();
    for (int32 i = 0; i < ChildCount; ++i)
    {
        UWidget* StarWidget = WantedStarsBox->GetChildAt(i);
        if (StarWidget)
        {
            StarWidget->SetVisibility(i < Stars
                ? ESlateVisibility::Visible
                : ESlateVisibility::Hidden);
        }
    }
}

void URealWorldMobileHUD::UpdateMinimapRotation(float HeadingDegrees)
{
    if (MinimapImage)
    {
        // Rotate minimap texture inversely to player heading (map stays fixed, arrow rotates)
        MinimapImage->SetRenderTransformAngle(-HeadingDegrees);
    }
}

void URealWorldMobileHUD::ShowWeaponWheel(bool bShow)
{
    if (WeaponWheelOverlay)
    {
        WeaponWheelOverlay->SetVisibility(bShow
            ? ESlateVisibility::Visible
            : ESlateVisibility::Hidden);
    }
}
