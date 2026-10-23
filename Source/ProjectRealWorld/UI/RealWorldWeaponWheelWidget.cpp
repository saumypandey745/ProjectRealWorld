// ============================================================================
// RealWorldWeaponWheelWidget.cpp
// Project RealWorld — GTA V-Class 8-Slot Radial Weapon Wheel
// ============================================================================

#include "UI/RealWorldWeaponWheelWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void URealWorldWeaponWheelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Default weapon slots
    SlotsData.Empty();

    auto AddDefaultSlot = [this](EWeaponWheelSlot Slot, const FString& Name, bool bUnlocked)
    {
        FWeaponWheelSlotData Data;
        Data.SlotType = Slot;
        Data.DisplayName = FText::FromString(Name);
        Data.bIsUnlocked = bUnlocked;
        SlotsData.Add(Slot, Data);
    };

    AddDefaultSlot(EWeaponWheelSlot::Unarmed, TEXT("Unarmed (Fists)"), true);
    AddDefaultSlot(EWeaponWheelSlot::Pistol,  TEXT("9mm Combat Pistol"), true);
    AddDefaultSlot(EWeaponWheelSlot::SMG,     TEXT("Micro SMG"), true);
    AddDefaultSlot(EWeaponWheelSlot::Shotgun, TEXT("Pump Shotgun"), true);
    AddDefaultSlot(EWeaponWheelSlot::Rifle,   TEXT("Carbine Assault Rifle"), true);
    AddDefaultSlot(EWeaponWheelSlot::Sniper,  TEXT("Heavy Sniper"), false);
    AddDefaultSlot(EWeaponWheelSlot::Heavy,   TEXT("RPG Rocket Launcher"), true);
    AddDefaultSlot(EWeaponWheelSlot::Thrown,  TEXT("Frag Grenade"), true);
}

void URealWorldWeaponWheelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

void URealWorldWeaponWheelWidget::OpenWheel()
{
    if (bIsOpen) return;
    bIsOpen = true;

    SetVisibility(ESlateVisibility::Visible);

    // GTA V Bullet-time slow motion
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGlobalTimeDilation(World, 0.2f);
    }

    if (WheelOpenSound)
    {
        UGameplayStatics::PlaySound2D(this, WheelOpenSound);
    }
}

void URealWorldWeaponWheelWidget::CloseWheel()
{
    if (!bIsOpen) return;
    bIsOpen = false;

    SetVisibility(ESlateVisibility::Collapsed);

    // Restore full speed game time
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
    }

    // Broadcast selected weapon slot
    OnWeaponWheelSlotSelected.Broadcast(ActiveSlot);
}

void URealWorldWeaponWheelWidget::UpdateTouchVector(const FVector2D& TouchInput)
{
    if (TouchInput.SizeSquared() < 0.2f) return;

    // Convert vector to angle: 0 degrees = Up (North)
    float Angle = FMath::RadiansToDegrees(FMath::Atan2(TouchInput.X, -TouchInput.Y));
    if (Angle < 0.0f) Angle += 360.0f;

    SelectSlotFromAngle(Angle);
}

void URealWorldWeaponWheelWidget::SelectSlotFromAngle(float AngleDegrees)
{
    // 8 slots = 45 degrees per slot
    // Offset by 22.5 to center the top slot at 0 deg
    const float ShiftedAngle = FMath::Fmod(AngleDegrees + 22.5f, 360.0f);
    const int32 SectorIndex = FMath::Clamp(FMath::FloorToInt(ShiftedAngle / 45.0f), 0, 7);

    static const EWeaponWheelSlot SlotsMap[8] = {
        EWeaponWheelSlot::Rifle,    // Top (North)
        EWeaponWheelSlot::Sniper,   // North-East
        EWeaponWheelSlot::Heavy,    // East
        EWeaponWheelSlot::Thrown,   // South-East
        EWeaponWheelSlot::Unarmed,  // South
        EWeaponWheelSlot::Pistol,   // South-West
        EWeaponWheelSlot::SMG,      // West
        EWeaponWheelSlot::Shotgun   // North-West
    };

    const EWeaponWheelSlot NewSlot = SlotsMap[SectorIndex];
    if (NewSlot != ActiveSlot)
    {
        ActiveSlot = NewSlot;
        if (SlotTickSound)
        {
            UGameplayStatics::PlaySound2D(this, SlotTickSound, 0.6f);
        }
    }
}

void URealWorldWeaponWheelWidget::SetSlotData(EWeaponWheelSlot InSlot, const FWeaponWheelSlotData& Data)
{
    SlotsData.Add(InSlot, Data);
}

FWeaponWheelSlotData URealWorldWeaponWheelWidget::GetSlotData(EWeaponWheelSlot InSlot) const
{
    if (const FWeaponWheelSlotData* Found = SlotsData.Find(InSlot))
    {
        return *Found;
    }
    return FWeaponWheelSlotData();
}
