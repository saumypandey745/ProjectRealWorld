#include "Core/RealWorldPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

ARealWorldPlayerController::ARealWorldPlayerController()
{
	bShowMouseCursor = false;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void ARealWorldPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Set input mode to Game Only by default
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void ARealWorldPlayerController::SwitchToVehicleContext(UInputMappingContext* VehicleContext)
{
	if (!VehicleContext) return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (CurrentContext)
		{
			Subsystem->RemoveMappingContext(CurrentContext);
		}
		Subsystem->AddMappingContext(VehicleContext, 1);
		CurrentContext = VehicleContext;
	}
}

void ARealWorldPlayerController::SwitchToOnFootContext(UInputMappingContext* OnFootContext)
{
	if (!OnFootContext) return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (CurrentContext)
		{
			Subsystem->RemoveMappingContext(CurrentContext);
		}
		Subsystem->AddMappingContext(OnFootContext, 0);
		CurrentContext = OnFootContext;
	}
}
