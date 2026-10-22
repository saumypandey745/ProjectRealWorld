// ============================================================================
// RealWorldPayNSprayGarage.cpp
// Project RealWorld — GTA5-class Pay 'n' Spray Vehicle Repair & Respray Garage
// ============================================================================

#include "Stores/RealWorldPayNSprayGarage.h"
#include "Components/BoxComponent.h"
#include "Vehicles/RealWorldVehicleBase.h"
#include "Vehicles/RealWorldVehicleDamageComponent.h"
#include "Subsystems/PoliceWantedSubsystem.h"
#include "Kismet/GameplayStatics.h"

ARealWorldPayNSprayGarage::ARealWorldPayNSprayGarage()
{
    PrimaryActorTick.bCanEverTick = false;

    GarageBayTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("GarageBayTrigger"));
    RootComponent = GarageBayTrigger;
    GarageBayTrigger->SetBoxExtent(FVector(500.0f, 400.0f, 250.0f));
    GarageBayTrigger->SetCollisionProfileName(TEXT("Trigger"));

    ServiceFee = 250.0f;
}

void ARealWorldPayNSprayGarage::BeginPlay()
{
    Super::BeginPlay();

    GarageBayTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARealWorldPayNSprayGarage::OnGarageBayOverlap);
    InitializeColorPalette();
}

void ARealWorldPayNSprayGarage::InitializeColorPalette()
{
    PaintPalette.Empty();
    PaintPalette.Add(FLinearColor(0.02f, 0.02f, 0.02f));  // Midnight Black
    PaintPalette.Add(FLinearColor(0.85f, 0.05f, 0.05f));  // Torred Racing Red
    PaintPalette.Add(FLinearColor(0.05f, 0.15f, 0.75f));  // Deep Royal Blue
    PaintPalette.Add(FLinearColor(0.95f, 0.95f, 0.95f));  // Pearl White
    PaintPalette.Add(FLinearColor(0.92f, 0.72f, 0.12f));  // Solace Gold
    PaintPalette.Add(FLinearColor(0.12f, 0.55f, 0.22f));  // British Racing Green
    PaintPalette.Add(FLinearColor(0.45f, 0.05f, 0.65f));  // Syndicate Purple
}

bool ARealWorldPayNSprayGarage::ServiceVehicle(ARealWorldVehicleBase* Vehicle, int32 ColorIndex)
{
    if (!Vehicle) return false;

    // 1. Repair all vehicle physical damage and tyre bursts
    if (URealWorldVehicleDamageComponent* DamageComp = Vehicle->FindComponentByClass<URealWorldVehicleDamageComponent>())
    {
        DamageComp->RepairVehicle();
    }

    // 2. Pick new color from palette
    FLinearColor ChosenColor = FLinearColor(0.05f, 0.15f, 0.75f); // default blue
    if (PaintPalette.IsValidIndex(ColorIndex))
    {
        ChosenColor = PaintPalette[ColorIndex];
    }
    else if (PaintPalette.Num() > 0)
    {
        const int32 RandomIdx = FMath::RandRange(0, PaintPalette.Num() - 1);
        ChosenColor = PaintPalette[RandomIdx];
    }

    // 3. WIPE POLICE WANTED LEVEL (The GTA classic feature)
    if (UWorld* World = GetWorld())
    {
        if (UPoliceWantedSubsystem* WantedSub = World->GetSubsystem<UPoliceWantedSubsystem>())
        {
            WantedSub->WantedLevel = 0;
            WantedSub->CurrentPursuitState = EPursuitState::Calm;
            WantedSub->CrimeSeverityAccumulator = 0.0f;
        }
    }

    // 4. Play spray sound & VFX
    if (SprayPaintSound && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(this, SprayPaintSound, GetActorLocation());
    }

    OnVehicleResprayed.Broadcast(Vehicle, ChosenColor);
    return true;
}

void ARealWorldPayNSprayGarage::OnGarageBayOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                                   bool bFromSweep, const FHitResult& SweepResult)
{
    if (ARealWorldVehicleBase* Vehicle = Cast<ARealWorldVehicleBase>(OtherActor))
    {
        // Auto service vehicle upon driving into bay
        ServiceVehicle(Vehicle, -1);
    }
}
