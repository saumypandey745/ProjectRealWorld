#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RealWorldPlayerController.generated.h"

class UInputMappingContext;
class ARealWorldHUD;

UCLASS()
class PROJECTREALWORLD_API ARealWorldPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARealWorldPlayerController();

	/** Switch between on-foot and vehicle control mapping contexts */
	UFUNCTION(BlueprintCallable, Category = "InputContext")
	void SwitchToVehicleContext(UInputMappingContext* VehicleContext);

	UFUNCTION(BlueprintCallable, Category = "InputContext")
	void SwitchToOnFootContext(UInputMappingContext* OnFootContext);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UInputMappingContext> CurrentContext;
};
