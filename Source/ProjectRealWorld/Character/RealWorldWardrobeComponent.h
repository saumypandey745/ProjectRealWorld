// ============================================================================
// RealWorldWardrobeComponent.h
// Project RealWorld — GTA V-Class Outfits, Wardrobe & Mask Disguise System
// Features: Dynamic clothing meshes, tactical heist masks,
//           police recognition delay, line-of-sight disguise breaking
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "RealWorldWardrobeComponent.generated.h"

class USkeletalMesh;
class UStaticMesh;
class ACharacter;

UENUM(BlueprintType)
enum class EOutfitCategory : uint8
{
    CasualStreet    UMETA(DisplayName = "Casual Streetwear (Hoodie/Jeans)"),
    TacticalHeist   UMETA(DisplayName = "Tactical Heist (Kevlar/Combat Boots)"),
    LuxurySuit      UMETA(DisplayName = "Luxury High-End Suit"),
    BikerLeather    UMETA(DisplayName = "Biker Leather Jacket"),
    Beachwear       UMETA(DisplayName = "Beach Shorts / Swimwear"),
};

UENUM(BlueprintType)
enum class EFaceMaskType : uint8
{
    None            UMETA(DisplayName = "No Mask (Recognizable)"),
    Balaclava       UMETA(DisplayName = "Tactical Balaclava"),
    ClownMask       UMETA(DisplayName = "Clown Heist Mask"),
    SkullBandana    UMETA(DisplayName = "Skull Bandana"),
    GasMask         UMETA(DisplayName = "Military Gas Mask"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOutfitChanged, EOutfitCategory, NewOutfit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaskToggled, EFaceMaskType, NewMask);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREALWORLD_API URealWorldWardrobeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URealWorldWardrobeComponent();

    virtual void BeginPlay() override;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Wardrobe|Actions")
    void SetOutfit(EOutfitCategory NewOutfit);

    UFUNCTION(BlueprintCallable, Category = "Wardrobe|Actions")
    void SetMask(EFaceMaskType NewMask);

    UFUNCTION(BlueprintCallable, Category = "Wardrobe|Actions")
    void RemoveMask();

    // ─── Queries ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "Wardrobe|State")
    EOutfitCategory GetCurrentOutfit() const { return CurrentOutfit; }

    UFUNCTION(BlueprintPure, Category = "Wardrobe|State")
    EFaceMaskType GetCurrentMask() const { return CurrentMask; }

    UFUNCTION(BlueprintPure, Category = "Wardrobe|State")
    bool IsWearingMask() const { return CurrentMask != EFaceMaskType::None; }

    UFUNCTION(BlueprintPure, Category = "Wardrobe|State")
    float GetPoliceRecognitionMultiplier() const;

    // ─── SFX Assets ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Audio")
    TObjectPtr<USoundBase> ClothesRustleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Audio")
    TObjectPtr<USoundBase> MaskEquipSound;

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Wardrobe|Events")
    FOnOutfitChanged OnOutfitChanged;

    UPROPERTY(BlueprintAssignable, Category = "Wardrobe|Events")
    FOnMaskToggled OnMaskToggled;

private:
    EOutfitCategory CurrentOutfit = EOutfitCategory::CasualStreet;
    EFaceMaskType CurrentMask = EFaceMaskType::None;

    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;
};
