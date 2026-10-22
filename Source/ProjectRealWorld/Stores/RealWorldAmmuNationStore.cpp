// ============================================================================
// RealWorldAmmuNationStore.cpp
// Project RealWorld — GTA5-class Ammu-Nation Gun Store
// ============================================================================

#include "Stores/RealWorldAmmuNationStore.h"
#include "Components/BoxComponent.h"
#include "Character/RealWorldCharacter.h"
#include "Character/RealWorldHealthComponent.h"
#include "Combat/RealWorldCombatComponent.h"
#include "Kismet/GameplayStatics.h"

ARealWorldAmmuNationStore::ARealWorldAmmuNationStore()
{
    PrimaryActorTick.bCanEverTick = false;

    StoreTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("StoreTriggerBox"));
    RootComponent = StoreTriggerBox;
    StoreTriggerBox->SetBoxExtent(FVector(300.0f, 300.0f, 150.0f));
    StoreTriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

void ARealWorldAmmuNationStore::BeginPlay()
{
    Super::BeginPlay();

    StoreTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ARealWorldAmmuNationStore::OnOverlapBegin);
    StoreTriggerBox->OnComponentEndOverlap.AddDynamic(this, &ARealWorldAmmuNationStore::OnOverlapEnd);

    BuildDefaultCatalog();
}

void ARealWorldAmmuNationStore::BuildDefaultCatalog()
{
    Catalog.Empty();

    // 1: Combat Pistol 9mm
    {
        FAmmuNationItem Item;
        Item.ItemID = TEXT("Wep_Pistol");
        Item.ItemName = FText::FromString(TEXT("Combat Pistol .45"));
        Item.Description = FText::FromString(TEXT("Reliable semi-automatic sidearm with high close-range stopping power."));
        Item.Price = 450.0f;
        Catalog.Add(Item);
    }
    // 2: Micro SMG
    {
        FAmmuNationItem Item;
        Item.ItemID = TEXT("Wep_SMG");
        Item.ItemName = FText::FromString(TEXT("Micro SMG 9mm"));
        Item.Description = FText::FromString(TEXT("High rate-of-fire compact automatic weapon. Perfect for drive-by shooting."));
        Item.Price = 1250.0f;
        Catalog.Add(Item);
    }
    // 3: Assault Rifle AK-47
    {
        FAmmuNationItem Item;
        Item.ItemID = TEXT("Wep_AssaultRifle");
        Item.ItemName = FText::FromString(TEXT("Assault Rifle 7.62mm"));
        Item.Description = FText::FromString(TEXT("Standard military-grade rifle with superior range, accuracy, and armor penetration."));
        Item.Price = 3500.0f;
        Catalog.Add(Item);
    }
    // 4: Pump Shotgun
    {
        FAmmuNationItem Item;
        Item.ItemID = TEXT("Wep_Shotgun");
        Item.ItemName = FText::FromString(TEXT("Pump-Action Shotgun 12G"));
        Item.Description = FText::FromString(TEXT("Devastating spread damage at close quarters. Blows doors and obstacles off hinges."));
        Item.Price = 2000.0f;
        Catalog.Add(Item);
    }
    // 5: Light Kevlar Body Armor
    {
        FAmmuNationItem Item;
        Item.ItemID = TEXT("Armor_Light");
        Item.ItemName = FText::FromString(TEXT("Standard Kevlar Armor (50 AP)"));
        Item.Description = FText::FromString(TEXT("Absorbs 65% of ballistic damage until depleted."));
        Item.Price = 300.0f;
        Item.bIsArmor = true;
        Item.ArmorAmount = 50.0f;
        Catalog.Add(Item);
    }
    // 6: Heavy Tactical Armor
    {
        FAmmuNationItem Item;
        Item.ItemID = TEXT("Armor_Heavy");
        Item.ItemName = FText::FromString(TEXT("Heavy SWAT Vest (100 AP)"));
        Item.Description = FText::FromString(TEXT("Maximum ballistic protection for high-heat firefights."));
        Item.Price = 800.0f;
        Item.bIsArmor = true;
        Item.ArmorAmount = 100.0f;
        Catalog.Add(Item);
    }
    // 7: Universal Ammo Crate
    {
        FAmmuNationItem Item;
        Item.ItemID = TEXT("Ammo_Universal");
        Item.ItemName = FText::FromString(TEXT("High-Grain Ammo Crate (+120 rounds)"));
        Item.Description = FText::FromString(TEXT("Replenishes reserve magazines for all equipped firearms."));
        Item.Price = 150.0f;
        Item.bIsAmmoRefill = true;
        Item.AmmoAmount = 120;
        Catalog.Add(Item);
    }
}

bool ARealWorldAmmuNationStore::PurchaseItem(ARealWorldCharacter* Buyer, FName ItemID)
{
    if (!Buyer) return false;

    const FAmmuNationItem* FoundItem = Catalog.FindByPredicate(
        [&](const FAmmuNationItem& Item) { return Item.ItemID == ItemID; }
    );

    if (!FoundItem) return false;

    // Apply Armor
    if (FoundItem->bIsArmor)
    {
        if (URealWorldHealthComponent* HealthComp = Buyer->FindComponentByClass<URealWorldHealthComponent>())
        {
            HealthComp->AddArmor(FoundItem->ArmorAmount);
        }
    }
    // Apply Ammo
    else if (FoundItem->bIsAmmoRefill)
    {
        if (URealWorldCombatComponent* CombatComp = Buyer->FindComponentByClass<URealWorldCombatComponent>())
        {
            if (CombatComp->CurrentWeapon)
            {
                CombatComp->CurrentWeapon->TotalReserveAmmo += FoundItem->AmmoAmount;
            }
        }
    }
    // Equip Weapon
    else if (FoundItem->WeaponClass)
    {
        if (URealWorldCombatComponent* CombatComp = Buyer->FindComponentByClass<URealWorldCombatComponent>())
        {
            CombatComp->EquipWeapon(FoundItem->WeaponClass);
        }
    }

    if (CashRegisterSound && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(this, CashRegisterSound, GetActorLocation());
    }

    OnItemPurchased.Broadcast(*FoundItem, true);
    return true;
}

void ARealWorldAmmuNationStore::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                               bool bFromSweep, const FHitResult& SweepResult)
{
    if (Cast<ARealWorldCharacter>(OtherActor))
    {
        // Player entered store
    }
}

void ARealWorldAmmuNationStore::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}
