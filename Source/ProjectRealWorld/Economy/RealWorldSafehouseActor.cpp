// ============================================================================
// RealWorldSafehouseActor.cpp
// Project RealWorld — GTA5-class Buyable Safehouses & Garages (Real Estate)
// ============================================================================

#include "Economy/RealWorldSafehouseActor.h"
#include "Components/BoxComponent.h"
#include "Character/RealWorldCharacter.h"
#include "Character/RealWorldHealthComponent.h"
#include "Subsystems/TimeOfDaySubsystem.h"
#include "Core/RealWorldSaveGame.h"
#include "Kismet/GameplayStatics.h"

ARealWorldSafehouseActor::ARealWorldSafehouseActor()
{
    PrimaryActorTick.bCanEverTick = false;

    PropertyBoundary = CreateDefaultSubobject<UBoxComponent>(TEXT("PropertyBoundary"));
    RootComponent = PropertyBoundary;
    PropertyBoundary->SetBoxExtent(FVector(800.0f, 800.0f, 300.0f));
    PropertyBoundary->SetCollisionProfileName(TEXT("Trigger"));

    PurchasePrice = 45000.0f;
    bIsOwned = false;
    GarageCapacity = 2;
    SleepTimeAdvanceHours = 6.0f;
}

void ARealWorldSafehouseActor::BeginPlay()
{
    Super::BeginPlay();
}

bool ARealWorldSafehouseActor::PurchaseProperty(ARealWorldCharacter* Buyer)
{
    if (!Buyer || bIsOwned) return false;

    bIsOwned = true;

    if (PurchaseSuccessAudio && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(this, PurchaseSuccessAudio, GetActorLocation());
    }

    OnSafehousePurchased.Broadcast(SafehouseID);
    return true;
}

void ARealWorldSafehouseActor::SleepAndSave(ARealWorldCharacter* Player)
{
    if (!Player || !bIsOwned) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // 1. Advance in-game time by 6 hours
    if (UTimeOfDaySubsystem* TimeSub = World->GetSubsystem<UTimeOfDaySubsystem>())
    {
        TimeSub->CurrentTimeHours = FMath::Fmod(TimeSub->CurrentTimeHours + SleepTimeAdvanceHours, 24.0f);
    }

    // 2. Full health restoration
    if (URealWorldHealthComponent* Health = Player->FindComponentByClass<URealWorldHealthComponent>())
    {
        Health->CurrentHealth = Health->MaxHealth;
    }

    // 3. Save game to disk
    if (URealWorldSaveGame* SaveObj = Cast<URealWorldSaveGame>(
        UGameplayStatics::CreateSaveGameObject(URealWorldSaveGame::StaticClass())))
    {
        SaveObj->PlayerTransform = Player->GetActorTransform();
        SaveObj->Timestamp = FDateTime::Now();
        UGameplayStatics::SaveGameToSlot(SaveObj, TEXT("Slot_01"), 0);
    }

    if (AlarmClockSound && World)
    {
        UGameplayStatics::PlaySound2D(World, AlarmClockSound);
    }

    OnPlayerSlept.Broadcast();
}
