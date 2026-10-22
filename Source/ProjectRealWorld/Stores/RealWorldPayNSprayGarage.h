// ============================================================================
// RealWorldPayNSprayGarage.h
// Project RealWorld — GTA5-class Pay 'n' Spray Vehicle Repair & Respray Garage
// Repairs body deformation, restores engine/tires, clears Police Wanted Level
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealWorldPayNSprayGarage.generated.h"

class UBoxComponent;
class ARealWorldVehicleBase;
class USoundBase;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleResprayed, ARealWorldVehicleBase*, Vehicle, FLinearColor, NewColor);

UCLASS(BlueprintType, Blueprintable)
class PROJECTREALWORLD_API ARealWorldPayNSprayGarage : public AActor
{
    GENERATED_BODY()

public:
    ARealWorldPayNSprayGarage();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Garage|Components")
    TObjectPtr<UBoxComponent> GarageBayTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garage|Config")
    float ServiceFee = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garage|Config")
    TArray<FLinearColor> PaintPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garage|Audio")
    TObjectPtr<USoundBase> SprayPaintSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garage|Audio")
    TObjectPtr<USoundBase> GarageDoorCloseSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garage|VFX")
    TObjectPtr<UNiagaraSystem> PaintMistFX;

    // ─── Service ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Garage|Actions")
    bool ServiceVehicle(ARealWorldVehicleBase* Vehicle, int32 ColorIndex);

    UPROPERTY(BlueprintAssignable, Category = "Garage|Events")
    FOnVehicleResprayed OnVehicleResprayed;

protected:
    UFUNCTION()
    void OnGarageBayOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                            bool bFromSweep, const FHitResult& SweepResult);

    void InitializeColorPalette();
};
