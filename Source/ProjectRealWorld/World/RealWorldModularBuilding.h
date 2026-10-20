#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealWorldModularBuilding.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UBoxComponent;
class ARealWorldDoor;

UCLASS()
class PROJECTREALWORLD_API ARealWorldModularBuilding : public AActor
{
	GENERATED_BODY()

public:
	ARealWorldModularBuilding();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building|Root")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Number of upper residential/office floors above ground floor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Config")
	int32 UpperFloorCount = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building|Trigger")
	TObjectPtr<UBoxComponent> InteriorThresholdTrigger;

	/** Construct modular architecture and detailed interior */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Building|Actions")
	void AssembleModularBuilding();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnInteriorTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteriorTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void BuildExteriorShell();
	void BuildCommercialInterior();
	void BuildRooftopHVAC();
};
