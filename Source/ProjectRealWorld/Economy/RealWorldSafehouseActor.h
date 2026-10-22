// ============================================================================
// RealWorldSafehouseActor.h
// Project RealWorld — GTA5-class Buyable Safehouses & Garages (Real Estate)
// Purchase apartments, quick sleep time advance, health restore, and car garage
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealWorldSafehouseActor.generated.h"

class UBoxComponent;
class ARealWorldCharacter;
class ARealWorldVehicleBase;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSafehousePurchased, FName, SafehouseID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerSlept);

UCLASS(BlueprintType, Blueprintable)
class PROJECTREALWORLD_API ARealWorldSafehouseActor : public AActor
{
    GENERATED_BODY()

public:
    ARealWorldSafehouseActor();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Safehouse|Components")
    TObjectPtr<UBoxComponent> PropertyBoundary;

    // ─── Property Details ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safehouse|Details")
    FName SafehouseID = TEXT("Safehouse_Downtown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safehouse|Details")
    FText PropertyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safehouse|Details")
    FText DistrictLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safehouse|Details")
    float PurchasePrice = 45000.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Safehouse|State")
    bool bIsOwned = false;

    // ─── Garage & Bed ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safehouse|Garage")
    int32 GarageCapacity = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safehouse|Sleep")
    float SleepTimeAdvanceHours = 6.0f; // Sleep 6 hours

    // ─── Actions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Safehouse|Actions")
    bool PurchaseProperty(ARealWorldCharacter* Buyer);

    UFUNCTION(BlueprintCallable, Category = "Safehouse|Actions")
    void SleepAndSave(ARealWorldCharacter* Player);

    UPROPERTY(BlueprintAssignable, Category = "Safehouse|Events")
    FOnSafehousePurchased OnSafehousePurchased;

    UPROPERTY(BlueprintAssignable, Category = "Safehouse|Events")
    FOnPlayerSlept OnPlayerSlept;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safehouse|Audio")
    TObjectPtr<USoundBase> PurchaseSuccessAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safehouse|Audio")
    TObjectPtr<USoundBase> AlarmClockSound;
};
