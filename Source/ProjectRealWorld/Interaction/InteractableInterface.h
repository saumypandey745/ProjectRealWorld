#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

class ARealWorldCharacter;

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for all interactive actors in Project RealWorld (Doors, Vehicles, Terminals, NPCs, Shops).
 */
class PROJECTREALWORLD_API IInteractableInterface
{
	GENERATED_BODY()

public:
	/** Get human-readable interaction prompt (e.g. "Open Door", "Enter Vehicle", "Talk") */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionPrompt(ARealWorldCharacter* InstigatorCharacter);

	/** Verify if interaction is currently valid */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(ARealWorldCharacter* InstigatorCharacter);

	/** Execute interaction logic */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(ARealWorldCharacter* InstigatorCharacter);
};
