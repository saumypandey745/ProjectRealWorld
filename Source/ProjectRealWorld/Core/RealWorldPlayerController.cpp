#include "Core/RealWorldPlayerController.h"
#include "Platform/RealWorldPlatformSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

ARealWorldPlayerController::ARealWorldPlayerController()
{
	bShowMouseCursor = false;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	bIsUsingMobileTouchInput = false;
	bIsInVehicleMode = false;
}

void ARealWorldPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Query centralized platform subsystem to initialize correct input mode
	bool bShouldUseTouch = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (URealWorldPlatformSubsystem* PlatformSub = GI->GetSubsystem<URealWorldPlatformSubsystem>())
		{
			bShouldUseTouch = PlatformSub->IsMobilePlatform() || PlatformSub->SupportsTouchControls();
		}
	}

	SetInputPlatformMode(bShouldUseTouch);

	// Initial on-foot context bind
	SwitchToOnFootContext();
}

void ARealWorldPlayerController::SetInputPlatformMode(bool bUseMobileTouch)
{
	bIsUsingMobileTouchInput = bUseMobileTouch;

	if (bIsUsingMobileTouchInput)
	{
		// Touch setup: Game and UI mode allowing on-screen virtual controls
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
		bShowMouseCursor = false;
		bEnableTouchEvents = true;
	}
	else
	{
		// PC setup: Game Only mode
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
		bEnableTouchEvents = false;
	}

	// Reapply current context
	if (bIsInVehicleMode)
	{
		SwitchToVehicleContext();
	}
	else
	{
		SwitchToOnFootContext();
	}
}

void ARealWorldPlayerController::SwitchToVehicleContext(UInputMappingContext* VehicleContext)
{
	bIsInVehicleMode = true;

	UInputMappingContext* TargetContext = VehicleContext;
	if (!TargetContext)
	{
		TargetContext = bIsUsingMobileTouchInput ? Mobile_VehicleContext : PC_VehicleContext;
	}

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

	UInputMappingContext* TargetContext = OnFootContext;
	if (!TargetContext)
	{
		TargetContext = bIsUsingMobileTouchInput ? Mobile_OnFootContext : PC_OnFootContext;
	}

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
