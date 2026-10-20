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

	/** PC Input Mapping Contexts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|PC")
	TObjectPtr<UInputMappingContext> PC_OnFootContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|PC")
	TObjectPtr<UInputMappingContext> PC_VehicleContext;

	/** Mobile Touch Input Mapping Contexts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Mobile")
	TObjectPtr<UInputMappingContext> Mobile_OnFootContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Mobile")
	TObjectPtr<UInputMappingContext> Mobile_VehicleContext;

	/** Whether active input mode is mobile touch or PC keyboard/mouse/gamepad */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input|State")
	bool bIsUsingMobileTouchInput = false;

	/** Switch between on-foot and vehicle control mapping contexts */
	UFUNCTION(BlueprintCallable, Category = "InputContext")
	void SwitchToVehicleContext(UInputMappingContext* VehicleContext = nullptr);

	UFUNCTION(BlueprintCallable, Category = "InputContext")
	void SwitchToOnFootContext(UInputMappingContext* OnFootContext = nullptr);

	/** Switch input handling mode between Touch and Mouse/Keyboard/Gamepad */
	UFUNCTION(BlueprintCallable, Category = "Input|Platform")
	void SetInputPlatformMode(bool bUseMobileTouch);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UInputMappingContext> CurrentContext;

	bool bIsInVehicleMode = false;
};
