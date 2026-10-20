#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "RealWorldDoor.generated.h"

class UStaticMeshComponent;

UCLASS()
class PROJECTREALWORLD_API ARealWorldDoor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ARealWorldDoor();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|Mesh")
	TObjectPtr<UStaticMeshComponent> FrameMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|Mesh")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Config")
	bool bIsOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Config")
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Config")
	float OpenAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Config")
	float SwingSpeed = 4.0f;

	// --- IInteractableInterface ---
	virtual FText GetInteractionPrompt_Implementation(ARealWorldCharacter* InstigatorCharacter) override;
	virtual bool CanInteract_Implementation(ARealWorldCharacter* InstigatorCharacter) override;
	virtual void Interact_Implementation(ARealWorldCharacter* InstigatorCharacter) override;

protected:
	virtual void BeginPlay() override;

private:
	FRotator ClosedRotation = FRotator::ZeroRotator;
	FRotator OpenRotation = FRotator::ZeroRotator;
};
