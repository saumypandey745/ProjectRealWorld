#include "Core/RealWorldPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

ARealWorldPlayerController::ARealWorldPlayerController()
{
	bShowMouseCursor = false;
	DefaultMouseCursor = EMouseCursor::None;
	bEnableTouchEvents = true;
	bEnableClickEvents = true;
	bEnableTouchOverEvents = false;
	bEnableMouseOverEvents = false;
	bIsInVehicleMode = false;
}

void ARealWorldPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Android Mobile Touch Setup: Game and UI mode allowing on-screen virtual joysticks & touch buttons
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(true);
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	bEnableTouchEvents = true;

	// Initial on-foot mobile touch context bind
	SwitchToOnFootContext();
}

void ARealWorldPlayerController::SwitchToVehicleContext(UInputMappingContext* VehicleContext)
{
	bIsInVehicleMode = true;

	UInputMappingContext* TargetContext = VehicleContext ? VehicleContext : Mobile_VehicleContext.Get();
	if (!TargetContext) return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (CurrentContext)
		{
			Subsystem->RemoveMappingContext(CurrentContext);
		}
		Subsystem->AddMappingContext(TargetContext, 1);
		CurrentContext = TargetContext;
	}
}

void ARealWorldPlayerController::SwitchToOnFootContext(UInputMappingContext* OnFootContext)
{
	bIsInVehicleMode = false;

	UInputMappingContext* TargetContext = OnFootContext ? OnFootContext : Mobile_OnFootContext.Get();
	if (!TargetContext) return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (CurrentContext)
		{
			Subsystem->RemoveMappingContext(CurrentContext);
		}
		Subsystem->AddMappingContext(TargetContext, 0);
		CurrentContext = TargetContext;
	}
}
