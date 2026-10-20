#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealWorldStreetProps.generated.h"

class UStaticMeshComponent;
class USpotLightComponent;

UENUM(BlueprintType)
enum class EStreetPropType : uint8
{
	TrafficSign UMETA(DisplayName = "Traffic Sign"),
	StreetLight UMETA(DisplayName = "Street Light"),
	ParkingMeter UMETA(DisplayName = "Parking Meter"),
	Bench UMETA(DisplayName = "Park Bench"),
	TrashBin UMETA(DisplayName = "Sidewalk Trash Bin"),
	Dumpster UMETA(DisplayName = "Alley Dumpster"),
	FireHydrant UMETA(DisplayName = "Fire Hydrant"),
	UtilityBox UMETA(DisplayName = "Electrical Utility Box"),
	RoadBarrier UMETA(DisplayName = "Concrete Road Barrier"),
	TrafficCone UMETA(DisplayName = "Traffic Cone"),
	Billboard UMETA(DisplayName = "Rooftop Billboard"),
	ShopSign UMETA(DisplayName = "Illuminated Shop Sign"),
	Mailbox UMETA(DisplayName = "Post Mailbox"),
	Planter UMETA(DisplayName = "Sidewalk Shrub Planter")
};

UCLASS()
class PROJECTREALWORLD_API ARealWorldStreetProp : public AActor
{
	GENERATED_BODY()

public:
	ARealWorldStreetProp();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop|Mesh")
	TObjectPtr<UStaticMeshComponent> PropMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop|Light")
	TObjectPtr<USpotLightComponent> LightComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop|Config")
	EStreetPropType PropType = EStreetPropType::Bench;

	/** Toggle light component for nighttime illumination */
	UFUNCTION(BlueprintCallable, Category = "Prop|Light")
	void SetLightActive(bool bActive);

	/** Reconfigure mesh and collision according to prop type */
	UFUNCTION(BlueprintCallable, Category = "Prop|Config")
	void SetupPropDimensions();
};
