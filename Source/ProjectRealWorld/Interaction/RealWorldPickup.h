#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "RealWorldPickup.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EPickupType : uint8
{
	Cash UMETA(DisplayName = "Cash Stack"),
	Ammo UMETA(DisplayName = "Ammunition"),
	Medkit UMETA(DisplayName = "Medkit"),
	MissionItem UMETA(DisplayName = "Mission Item")
};

UCLASS()
class PROJECTREALWORLD_API ARealWorldPickup : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ARealWorldPickup();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup|Mesh")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup|Collision")
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Config")
	EPickupType PickupType = EPickupType::Cash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Config")
	float Value = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Config")
	FText ItemName;

	// --- IInteractableInterface ---
	virtual FText GetInteractionPrompt_Implementation(ARealWorldCharacter* InstigatorCharacter) override;
	virtual bool CanInteract_Implementation(ARealWorldCharacter* InstigatorCharacter) override;
	virtual void Interact_Implementation(ARealWorldCharacter* InstigatorCharacter) override;
};
