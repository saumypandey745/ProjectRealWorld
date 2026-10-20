#include "Vehicles/RealWorldVehicleBase.h"
#include "Character/RealWorldCharacter.h"
#include "Core/RealWorldPlayerController.h"
#include "Input/RealWorldInputConfig.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

ARealWorldVehicleBase::ARealWorldVehicleBase()
{
	PrimaryActorTick.bCanEverTick = true;

	VehicleCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("VehicleCameraBoom"));
	VehicleCameraBoom->SetupAttachment(RootComponent);
	VehicleCameraBoom->TargetArmLength = 520.0f;
	VehicleCameraBoom->SocketOffset = FVector(0.0f, 0.0f, 120.0f);
	VehicleCameraBoom->bEnableCameraLag = true;
	VehicleCameraBoom->CameraLagSpeed = 16.0f;
	VehicleCameraBoom->CameraRotationLagSpeed = 20.0f;
	VehicleCameraBoom->bUsePawnControlRotation = true;
	VehicleCameraBoom->bInheritPitch = true;
	VehicleCameraBoom->bInheritYaw = true;
	VehicleCameraBoom->bInheritRoll = false;

	VehicleCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VehicleCamera"));
	VehicleCamera->SetupAttachment(VehicleCameraBoom, USpringArmComponent::SocketName);
	VehicleCamera->bUsePawnControlRotation = false;
	VehicleCamera->FieldOfView = 90.0f;

	// Physical defaults
	Category = EVehicleType::Sedan;
	MaxVehicleHealth = 1000.0f;
	CurrentVehicleHealth = 1000.0f;
	DamageState = EVehicleDamageState::Pristine;
	FuelCapacity = 65.0f;
	CurrentFuel = 65.0f;
	FuelConsumptionRate = 0.008f;
	EngineRPM = 800.0f;
	bEngineRunning = false;
	MaxForwardSpeedKmH = 180.0f;
}

void ARealWorldVehicleBase::BeginPlay()
{
	Super::BeginPlay();

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetNotifyRigidBodyCollision(true);
		MeshComp->OnComponentHit.AddDynamic(this, &ARealWorldVehicleBase::OnVehicleHit);
	}
}

void ARealWorldVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCameraDynamics(DeltaTime);
	UpdateFuelAndEngine(DeltaTime);
	ReevaluateDamageState();
}

void ARealWorldVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IA_ThrottleBrake)
		{
			EnhancedInputComponent->BindAction(IA_ThrottleBrake, ETriggerEvent::Triggered, this, &ARealWorldVehicleBase::Input_ThrottleBrake);
			EnhancedInputComponent->BindAction(IA_ThrottleBrake, ETriggerEvent::Completed, this, &ARealWorldVehicleBase::Input_ThrottleBrake);
		}

		if (IA_Steering)
		{
			EnhancedInputComponent->BindAction(IA_Steering, ETriggerEvent::Triggered, this, &ARealWorldVehicleBase::Input_Steer);
			EnhancedInputComponent->BindAction(IA_Steering, ETriggerEvent::Completed, this, &ARealWorldVehicleBase::Input_Steer);
		}

		if (IA_Handbrake)
		{
			EnhancedInputComponent->BindAction(IA_Handbrake, ETriggerEvent::Started, this, &ARealWorldVehicleBase::Input_HandbrakeStarted);
			EnhancedInputComponent->BindAction(IA_Handbrake, ETriggerEvent::Completed, this, &ARealWorldVehicleBase::Input_HandbrakeStopped);
		}

		if (IA_ExitVehicle)
		{
			EnhancedInputComponent->BindAction(IA_ExitVehicle, ETriggerEvent::Started, this, &ARealWorldVehicleBase::Input_ExitVehicle);
		}
	}
}

void ARealWorldVehicleBase::Input_ThrottleBrake(const FInputActionValue& Value)
{
	UChaosWheeledVehicleMovementComponent* MoveComp = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
	if (!MoveComp) return;

	if (!bEngineRunning || CurrentFuel <= 0.0f || DamageState == EVehicleDamageState::Totaled)
	{
		MoveComp->SetThrottleInput(0.0f);
		MoveComp->SetBrakeInput(1.0f);
		return;
	}

	const float AxisValue = Value.Get<float>();
	const float CurrentSpeed = GetForwardSpeedKmH();

	if (AxisValue > 0.0f)
	{
		// Accelerate forward
		if (CurrentSpeed < -2.0f)
		{
			// Currently reversing: apply brake to stop reverse movement
			MoveComp->SetThrottleInput(0.0f);
			MoveComp->SetBrakeInput(AxisValue);
		}
		else
		{
			// Forward drive
			MoveComp->SetThrottleInput(AxisValue);
			MoveComp->SetBrakeInput(0.0f);
		}
	}
	else if (AxisValue < 0.0f)
	{
		// Negative input: Brake if moving forward, or reverse if stopped/reversing
		if (CurrentSpeed > 2.0f)
		{
			MoveComp->SetThrottleInput(0.0f);
			MoveComp->SetBrakeInput(FMath::Abs(AxisValue));
		}
		else
		{
			// Reverse gear drive
			MoveComp->SetThrottleInput(AxisValue); // Negative throttle drives reverse in Chaos
			MoveComp->SetBrakeInput(0.0f);
		}
	}
	else
	{
		// Neutral coasting
		MoveComp->SetThrottleInput(0.0f);
		MoveComp->SetBrakeInput(0.0f);
	}
}

void ARealWorldVehicleBase::Input_Steer(const FInputActionValue& Value)
{
	if (UChaosWheeledVehicleMovementComponent* MoveComp = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		MoveComp->SetSteeringInput(Value.Get<float>());
	}
}

void ARealWorldVehicleBase::Input_HandbrakeStarted()
{
	if (UChaosWheeledVehicleMovementComponent* MoveComp = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		MoveComp->SetHandbrakeInput(true);
	}
}

void ARealWorldVehicleBase::Input_HandbrakeStopped()
{
	if (UChaosWheeledVehicleMovementComponent* MoveComp = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		MoveComp->SetHandbrakeInput(false);
	}
}

void ARealWorldVehicleBase::Input_ExitVehicle()
{
	ExitVehicle();
}

void ARealWorldVehicleBase::StartEngine()
{
	if (CurrentFuel > 0.0f && DamageState != EVehicleDamageState::Totaled)
	{
		bEngineRunning = true;
		EngineRPM = 800.0f;
	}
}

void ARealWorldVehicleBase::StopEngine()
{
	bEngineRunning = false;
	EngineRPM = 0.0f;
	if (UChaosWheeledVehicleMovementComponent* MoveComp = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		MoveComp->SetThrottleInput(0.0f);
	}
}

bool ARealWorldVehicleBase::EnterVehicle(ARealWorldCharacter* InDriver)
{
	if (!InDriver || bIsOccupied || DamageState == EVehicleDamageState::Totaled)
	{
		return false;
	}

	CurrentDriver = InDriver;
	bIsOccupied = true;

	// Transition driver into vehicle state
	InDriver->CurrentLocomotionState = ELocomotionState::InVehicle;
	InDriver->SetActorHiddenInGame(true);
	InDriver->SetActorEnableCollision(false);
	InDriver->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);

	// Transfer possession to vehicle pawn
	if (ARealWorldPlayerController* PC = Cast<ARealWorldPlayerController>(InDriver->GetController()))
	{
		PC->UnPossess();
		PC->Possess(this);

		if (VehicleInputMappingContext)
		{
			PC->SwitchToVehicleContext(VehicleInputMappingContext);
		}
	}

	StartEngine();
	return true;
}

bool ARealWorldVehicleBase::ExitVehicle()
{
	if (!bIsOccupied || !CurrentDriver)
	{
		return false;
	}

	// Safety check: Cannot safely exit if traveling over 35 km/h without bail-out penalty
	const float Speed = FMath::Abs(GetForwardSpeedKmH());
	if (Speed > 35.0f)
	{
		// Bail out causes physical damage to driver
		CurrentDriver->ApplyHitReaction(GetActorForwardVector(), 35.0f);
	}

	ARealWorldCharacter* ExitingDriver = CurrentDriver;
	CurrentDriver = nullptr;
	bIsOccupied = false;

	// Detach driver and place beside vehicle on driver side
	ExitingDriver->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	const FVector ExitPosition = GetActorLocation() + (GetActorRightVector() * DriverDoorOffset.X) + FVector(0.0f, 0.0f, 40.0f);
	ExitingDriver->SetActorLocation(ExitPosition);
	ExitingDriver->SetActorHiddenInGame(false);
	ExitingDriver->SetActorEnableCollision(true);
	ExitingDriver->CurrentLocomotionState = ELocomotionState::Idle;

	// Restore possession to player character
	if (ARealWorldPlayerController* PC = Cast<ARealWorldPlayerController>(GetController()))
	{
		PC->UnPossess();
		PC->Possess(ExitingDriver);

		if (ExitingDriver->InputConfig && ExitingDriver->InputConfig->IMC_OnFoot)
		{
			PC->SwitchToOnFootContext(ExitingDriver->InputConfig->IMC_OnFoot);
		}
	}

	return true;
}

void ARealWorldVehicleBase::UpdateFuelAndEngine(float DeltaTime)
{
	if (!bEngineRunning)
	{
		EngineRPM = FMath::FInterpTo(EngineRPM, 0.0f, DeltaTime, 4.0f);
		return;
	}

	UChaosWheeledVehicleMovementComponent* MoveComp = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
	const float CurrentSpeed = FMath::Abs(GetForwardSpeedKmH());
	const float Throttle = MoveComp ? FMath::Abs(MoveComp->GetThrottleInput()) : 0.0f;

	// Fuel consumption: idle baseline + active throttle consumption
	const float ActiveConsumption = (FuelConsumptionRate * 0.1f) + (FuelConsumptionRate * Throttle * (1.0f + CurrentSpeed / 100.0f));
	CurrentFuel = FMath::Max(0.0f, CurrentFuel - (ActiveConsumption * DeltaTime));

	if (CurrentFuel <= 0.0f)
	{
		StopEngine();
		return;
	}

	// Compute realistic engine RPM
	const float TargetRPM = FMath::Clamp(800.0f + (Throttle * 4500.0f) + (CurrentSpeed * 25.0f), 800.0f, 6500.0f);
	EngineRPM = FMath::FInterpTo(EngineRPM, TargetRPM, DeltaTime, 6.0f);
}

void ARealWorldVehicleBase::UpdateCameraDynamics(float DeltaTime)
{
	if (!VehicleCameraBoom || !VehicleCamera) return;

	const float SpeedKmH = FMath::Abs(GetForwardSpeedKmH());
	const float SpeedRatio = FMath::Clamp(SpeedKmH / MaxForwardSpeedKmH, 0.0f, 1.0f);

	// Dynamic chase arm length: 520.0f at rest, up to 720.0f at max speed
	const float DesiredArmLength = 520.0f + (SpeedRatio * 200.0f);
	VehicleCameraBoom->TargetArmLength = FMath::FInterpTo(VehicleCameraBoom->TargetArmLength, DesiredArmLength, DeltaTime, 3.0f);

	// Dynamic FOV speed sensation: 90 deg up to 105 deg
	const float DesiredFOV = 90.0f + (SpeedRatio * 15.0f);
	VehicleCamera->FieldOfView = FMath::FInterpTo(VehicleCamera->FieldOfView, DesiredFOV, DeltaTime, 4.0f);
}

void ARealWorldVehicleBase::ApplyPhysicalImpactDamage(float ImpulseMagnitude, FVector ImpactPoint)
{
	// Convert impulse magnitude (kg*cm/s) to realistic damage
	// Impacts below 30,000 are low-speed bumps absorbed by bumper
	if (ImpulseMagnitude < 30000.0f) return;

	const float DamageAmount = (ImpulseMagnitude - 30000.0f) / 1500.0f;
	CurrentVehicleHealth = FMath::Max(0.0f, CurrentVehicleHealth - DamageAmount);

	ReevaluateDamageState();

	if (CurrentVehicleHealth <= 0.0f)
	{
		StopEngine();
	}
}

void ARealWorldVehicleBase::ReevaluateDamageState()
{
	const float HealthPercent = GetHealthPercent();

	if (HealthPercent <= 0.0f)
	{
		DamageState = EVehicleDamageState::Totaled;
	}
	else if (HealthPercent < 0.15f)
	{
		DamageState = EVehicleDamageState::CriticalFire;
	}
	else if (HealthPercent < 0.30f)
	{
		DamageState = EVehicleDamageState::EngineSmoking;
	}
	else if (HealthPercent < 0.50f)
	{
		DamageState = EVehicleDamageState::HeavyDamage;
	}
	else if (HealthPercent < 0.70f)
	{
		DamageState = EVehicleDamageState::ModerateDamage;
	}
	else if (HealthPercent < 0.90f)
	{
		DamageState = EVehicleDamageState::MinorDents;
	}
	else
	{
		DamageState = EVehicleDamageState::Pristine;
	}
}

void ARealWorldVehicleBase::OnVehicleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this)
	{
		const float ImpulseSize = NormalImpulse.Size();
		ApplyPhysicalImpactDamage(ImpulseSize, Hit.ImpactPoint);
	}
}

float ARealWorldVehicleBase::GetForwardSpeedKmH() const
{
	if (UChaosWheeledVehicleMovementComponent* MoveComp = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		return MoveComp->GetForwardSpeed() * 0.036f; // cm/s to km/h
	}
	return 0.0f;
}

float ARealWorldVehicleBase::GetFuelPercent() const
{
	return FuelCapacity > 0.0f ? FMath::Clamp(CurrentFuel / FuelCapacity, 0.0f, 1.0f) : 0.0f;
}

float ARealWorldVehicleBase::GetHealthPercent() const
{
	return MaxVehicleHealth > 0.0f ? FMath::Clamp(CurrentVehicleHealth / MaxVehicleHealth, 0.0f, 1.0f) : 0.0f;
}

int32 ARealWorldVehicleBase::GetCurrentGear() const
{
	if (UChaosWheeledVehicleMovementComponent* MoveComp = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		return MoveComp->GetCurrentGear();
	}
	return 0;
}

FText ARealWorldVehicleBase::GetInteractionPrompt_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	if (OwnershipData.bIsLocked)
	{
		return FText::FromString(TEXT("Vehicle Locked"));
	}
	if (DamageState == EVehicleDamageState::Totaled)
	{
		return FText::FromString(TEXT("Vehicle Destroyed"));
	}
	return FText::FromString(TEXT("Drive Vehicle [E]"));
}

bool ARealWorldVehicleBase::CanInteract_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	return !bIsOccupied && !OwnershipData.bIsLocked && DamageState != EVehicleDamageState::Totaled;
}

void ARealWorldVehicleBase::Interact_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	EnterVehicle(InstigatorCharacter);
}
