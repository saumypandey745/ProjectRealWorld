// ============================================================================
// RealWorldAmmuNationStore.h
// Project RealWorld — GTA5-class Ammu-Nation Gun Store
// Weapons, Attachments (Silencer, Extended Mag, Scope), Body Armor, Ammo
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/RealWorldWeaponBase.h"
#include "RealWorldAmmuNationStore.generated.h"

class UBoxComponent;
class ARealWorldCharacter;
class USoundBase;

USTRUCT(BlueprintType)
struct FAmmuNationItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
    FName ItemID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
    float Price = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
    TSubclassOf<ARealWorldWeaponBase> WeaponClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
    bool bIsArmor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
    float ArmorAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
    bool bIsAmmoRefill = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
    int32 AmmoAmount = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPurchased, const FAmmuNationItem&, Item, bool, bSuccess);

UCLASS(BlueprintType, Blueprintable)
class PROJECTREALWORLD_API ARealWorldAmmuNationStore : public AActor
{
    GENERATED_BODY()

public:
    ARealWorldAmmuNationStore();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Store|Components")
    TObjectPtr<UBoxComponent> StoreTriggerBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Catalog")
    TArray<FAmmuNationItem> Catalog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Audio")
    TObjectPtr<USoundBase> CashRegisterSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Audio")
    TObjectPtr<USoundBase> PurchaseDeclinedSound;

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Store|Actions")
    bool PurchaseItem(ARealWorldCharacter* Buyer, FName ItemID);

    UFUNCTION(BlueprintPure, Category = "Store|Catalog")
    const TArray<FAmmuNationItem>& GetCatalog() const { return Catalog; }

    UPROPERTY(BlueprintAssignable, Category = "Store|Events")
    FOnItemPurchased OnItemPurchased;

protected:
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void BuildDefaultCatalog();
};
