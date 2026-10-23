#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RealWorldPlayerController.generated.h"

class UInputMappingContext;
class ARealWorldHUD;

/**
 * ARealWorldPlayerController
 * Android Mobile Exclusive Player Controller.
 * Manages touch-based virtual controls (dual virtual joysticks, touch look swipe, action buttons).
 */
UCLASS()
class PROJECTREALWORLD_API ARealWorldPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARealWorldPlayerController();

	/** Mobile Touch Input Mapping Context for On-Foot locomotion and actions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Mobile")
	TObjectPtr<UInputMappingContext> Mobile_OnFootContext;

	/** Mobile Touch Input Mapping Context for Vehicle driving and controls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Mobile")
	TObjectPtr<UInputMappingContext> Mobile_VehicleContext;

	/** Switch to vehicle driving touch context */
	UFUNCTION(BlueprintCallable, Category = "InputContext")
	void SwitchToVehicleContext(UInputMappingContext* VehicleContext = nullptr);

	/** Switch to on-foot exploration touch context */
	UFUNCTION(BlueprintCallable, Category = "InputContext")
	void SwitchToOnFootContext(UInputMappingContext* OnFootContext = nullptr);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UInputMappingContext> CurrentContext;

	bool bIsInVehicleMode = false;
};
