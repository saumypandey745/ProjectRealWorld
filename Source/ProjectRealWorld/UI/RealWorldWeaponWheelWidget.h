// ============================================================================
// RealWorldWeaponWheelWidget.h
// Project RealWorld — GTA V-Class 8-Slot Radial Weapon Wheel
// Features: Bullet-time slow-mo (Time Dilation 0.2f), touch gesture angle,
//           weapon stats inspection, smooth radial HUD UI
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sound/SoundBase.h"
#include "RealWorldWeaponWheelWidget.generated.h"

UENUM(BlueprintType)
enum class EWeaponWheelSlot : uint8
{
    Unarmed     UMETA(DisplayName = "Fists / Melee"),
    Pistol      UMETA(DisplayName = "Pistols"),
    SMG         UMETA(DisplayName = "Submachine Guns"),
    Shotgun     UMETA(DisplayName = "Shotguns"),
    Rifle       UMETA(DisplayName = "Assault Rifles"),
    Sniper      UMETA(DisplayName = "Sniper Rifles"),
    Heavy       UMETA(DisplayName = "Heavy (RPG / Minigun)"),
    Thrown      UMETA(DisplayName = "Thrown / Explosives"),
};

USTRUCT(BlueprintType)
struct FWeaponWheelSlotData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeaponWheelSlot SlotType = EWeaponWheelSlot::Unarmed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentAmmo = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ReserveAmmo = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamagePercent = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FireRatePercent = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnlocked = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponWheelSlotSelected, EWeaponWheelSlot, SelectedSlot);

UCLASS(Abstract, Blueprintable)
class PROJECTREALWORLD_API URealWorldWeaponWheelWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "WeaponWheel|Actions")
    void OpenWheel();

    UFUNCTION(BlueprintCallable, Category = "WeaponWheel|Actions")
    void CloseWheel();

    UFUNCTION(BlueprintCallable, Category = "WeaponWheel|Actions")
    void UpdateTouchVector(const FVector2D& TouchInput);

    UFUNCTION(BlueprintCallable, Category = "WeaponWheel|Actions")
    void SetSlotData(EWeaponWheelSlot InSlot, const FWeaponWheelSlotData& Data);

    // ─── Queries ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "WeaponWheel|State")
    bool IsWheelOpen() const { return bIsOpen; }

    UFUNCTION(BlueprintPure, Category = "WeaponWheel|State")
    EWeaponWheelSlot GetCurrentSelectedSlot() const { return ActiveSlot; }

    UFUNCTION(BlueprintPure, Category = "WeaponWheel|State")
    FWeaponWheelSlotData GetSlotData(EWeaponWheelSlot InSlot) const;

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "WeaponWheel|Events")
    FOnWeaponWheelSlotSelected OnWeaponWheelSlotSelected;

    // ─── SFX Assets ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponWheel|Audio")
    TObjectPtr<USoundBase> SlotTickSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponWheel|Audio")
    TObjectPtr<USoundBase> WheelOpenSound;

private:
    bool bIsOpen = false;
    EWeaponWheelSlot ActiveSlot = EWeaponWheelSlot::Unarmed;

    UPROPERTY()
    TMap<EWeaponWheelSlot, FWeaponWheelSlotData> SlotsData;

    void SelectSlotFromAngle(float AngleDegrees);
};
