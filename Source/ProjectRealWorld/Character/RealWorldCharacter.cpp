#include "Character/RealWorldCharacter.h"
#include "Input/RealWorldInputConfig.h"
#include "Interaction/RealWorldInteractionComponent.h"
#include "Combat/RealWorldCombatComponent.h"
#include "MotionWarpingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Kismet/KismetMathLibrary.h"

ARealWorldCharacter::ARealWorldCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Human capsule dimensions: 180cm height, 34cm radius
	GetCapsuleComponent()->InitCapsuleSize(34.0f, 90.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	// Character movement setup
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	MoveComp->MaxWalkSpeed = JogSpeed;
	MoveComp->MaxAcceleration = BaseAcceleration;
	MoveComp->BrakingDecelerationWalking = BaseBrakingDeceleration;
	MoveComp->BrakingFrictionFactor = 1.0f;
	MoveComp->JumpZVelocity = 480.0f;
	MoveComp->AirControl = 0.35f;
	MoveComp->GravityScale = 1.35f;
	MoveComp->NavAgentProps.bCanCrouch = true;

	// --- Modular Skeletal Meshes (MetaHuman Standard Assembly) ---
	FaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FaceMesh"));
	FaceMesh->SetupAttachment(GetMesh());
	FaceMesh->SetLeaderPoseComponent(GetMesh());

	TorsoMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TorsoMesh"));
	TorsoMesh->SetupAttachment(GetMesh());
	TorsoMesh->SetLeaderPoseComponent(GetMesh());

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh"));
	LegsMesh->SetupAttachment(GetMesh());
	LegsMesh->SetLeaderPoseComponent(GetMesh());

	FeetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FeetMesh"));
	FeetMesh->SetupAttachment(GetMesh());
	FeetMesh->SetLeaderPoseComponent(GetMesh());

	HairMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairMesh->SetupAttachment(FaceMesh);
	HairMesh->SetLeaderPoseComponent(FaceMesh);

	AccessoriesMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AccessoriesMesh"));
	AccessoriesMesh->SetupAttachment(GetMesh());
	AccessoriesMesh->SetLeaderPoseComponent(GetMesh());

	// Camera Boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = DefaultArmLength;
	CameraBoom->SocketOffset = DefaultSocketOffset;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.0f;
	CameraBoom->CameraLagMaxDistance = 45.0f;
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->ProbeSize = 12.0f;
	CameraBoom->ProbeChannel = ECC_Camera;

	// Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->SetFieldOfView(DefaultFOV);

	// Contextual Interaction Component
	InteractionComponent = CreateDefaultSubobject<URealWorldInteractionComponent>(TEXT("InteractionComponent"));

	// Combat Component
	CombatComponent = CreateDefaultSubobject<URealWorldCombatComponent>(TEXT("CombatComponent"));

	// Motion Warping Component
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	TargetSpeed = JogSpeed;
	bIsHitReacting = false;
	HitReactionTimer = 0.0f;
}

void ARealWorldCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
	bIsExhausted = false;

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (InputConfig && InputConfig->IMC_OnFoot)
			{
				Subsystem->AddMappingContext(InputConfig->IMC_OnFoot, 0);
			}
		}
	}
}

void ARealWorldCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsHitReacting)
	{
		HitReactionTimer -= DeltaTime;
		if (HitReactionTimer <= 0.0f)
		{
			bIsHitReacting = false;
		}
	}

	UpdateLocomotionState(DeltaTime);
	UpdateCameraTransform(DeltaTime);
	UpdateStamina(DeltaTime);
	UpdateFootIK(DeltaTime);

	if (bIsVaultInProgress)
	{
		ProcessVaultMovement(DeltaTime);
	}
}

void ARealWorldCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent || !InputConfig) return;

	if (InputConfig->IA_Move)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Move, ETriggerEvent::Triggered, this, &ARealWorldCharacter::Input_Move);
	}

	if (InputConfig->IA_Look)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Look, ETriggerEvent::Triggered, this, &ARealWorldCharacter::Input_Look);
	}

	if (InputConfig->IA_LookMouse)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_LookMouse, ETriggerEvent::Triggered, this, &ARealWorldCharacter::Input_LookMouse);
	}

	if (InputConfig->IA_Jump)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Jump, ETriggerEvent::Started, this, &ARealWorldCharacter::Input_JumpStarted);
		EnhancedInputComponent->BindAction(InputConfig->IA_Jump, ETriggerEvent::Completed, this, &ARealWorldCharacter::Input_JumpStopped);
	}

	if (InputConfig->IA_Sprint)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Sprint, ETriggerEvent::Started, this, &ARealWorldCharacter::Input_SprintStarted);
		EnhancedInputComponent->BindAction(InputConfig->IA_Sprint, ETriggerEvent::Completed, this, &ARealWorldCharacter::Input_SprintStopped);
	}

	if (InputConfig->IA_Crouch)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Crouch, ETriggerEvent::Started, this, &ARealWorldCharacter::Input_CrouchToggle);
	}

	if (InputConfig->IA_Interact)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Interact, ETriggerEvent::Started, this, &ARealWorldCharacter::Input_Interact);
	}

	if (InputConfig->IA_Attack)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Attack, ETriggerEvent::Started, this, &ARealWorldCharacter::Input_AttackStarted);
	}

	if (InputConfig->IA_Aim)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Aim, ETriggerEvent::Started, this, &ARealWorldCharacter::Input_AimStarted);
		EnhancedInputComponent->BindAction(InputConfig->IA_Aim, ETriggerEvent::Completed, this, &ARealWorldCharacter::Input_AimStopped);
	}

	if (InputConfig->IA_Reload)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Reload, ETriggerEvent::Started, this, &ARealWorldCharacter::Input_ReloadStarted);
	}

	if (InputConfig->IA_Vault)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Vault, ETriggerEvent::Started, this, &ARealWorldCharacter::Input_VaultStarted);
	}
}

void ARealWorldCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	CurrentLocomotionState = ELocomotionState::Land;
	LandingTimer = LandingDuration;
}

void ARealWorldCharacter::ApplyCustomization(const FRealWorldCharacterCustomization& NewData)
{
	CustomizationData = NewData;

	auto LoadAndAssign = [](TSoftObjectPtr<USkeletalMesh> SoftMesh, USkeletalMeshComponent* Comp)
	{
		if (Comp && !SoftMesh.IsNull())
		{
			if (USkeletalMesh* Loaded = SoftMesh.LoadSynchronous())
			{
				Comp->SetSkeletalMesh(Loaded);
			}
		}
	};

	LoadAndAssign(CustomizationData.HeadMesh, FaceMesh);
	LoadAndAssign(CustomizationData.HairMesh, HairMesh);
	LoadAndAssign(CustomizationData.Clothing.TorsoMesh, TorsoMesh);
	LoadAndAssign(CustomizationData.Clothing.LegsMesh, LegsMesh);
	LoadAndAssign(CustomizationData.Clothing.FeetMesh, FeetMesh);
	LoadAndAssign(CustomizationData.Clothing.AccessoriesMesh, AccessoriesMesh);
}

void ARealWorldCharacter::ApplyHitReaction(const FVector& HitDirection, float Damage)
{
	CurrentHealth = FMath::Max(0.0f, CurrentHealth - Damage);
	bIsHitReacting = true;
	LastHitDirection = HitDirection.GetSafeNormal();
	HitReactionTimer = 0.35f;
}

void ARealWorldCharacter::Input_Move(const FInputActionValue& Value)
{
	if (bIsVaultInProgress) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ARealWorldCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X * CameraSensitivityX);
		AddControllerPitchInput(LookAxisVector.Y * CameraSensitivityY);
	}
}

void ARealWorldCharacter::Input_LookMouse(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X * CameraSensitivityX);
		AddControllerPitchInput(LookAxisVector.Y * CameraSensitivityY);
	}
}

void ARealWorldCharacter::Input_JumpStarted()
{
	if (bIsVaultInProgress) return;

	if (TryExecuteVaultOrMantle())
	{
		return;
	}

	ACharacter::Jump();
}

void ARealWorldCharacter::Input_JumpStopped()
{
	ACharacter::StopJumping();
}

void ARealWorldCharacter::Input_SprintStarted()
{
	if (!bIsExhausted && CurrentStamina > 10.0f)
	{
		bWantsToSprint = true;
	}
}

void ARealWorldCharacter::Input_SprintStopped()
{
	bWantsToSprint = false;
}

void ARealWorldCharacter::Input_CrouchToggle()
{
	if (bIsVaultInProgress) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ARealWorldCharacter::Input_Interact()
{
	if (InteractionComponent)
	{
		InteractionComponent->PerformInteraction();
	}
}

void ARealWorldCharacter::Input_AttackStarted()
{
	if (CombatComponent)
	{
		CombatComponent->FireCurrentWeapon();
	}
}

void ARealWorldCharacter::Input_AimStarted()
{
	SetAimingState(true);
}

void ARealWorldCharacter::Input_AimStopped()
{
	SetAimingState(false);
}

void ARealWorldCharacter::Input_ReloadStarted()
{
	if (CombatComponent)
	{
		CombatComponent->ReloadCurrentWeapon();
	}
}

void ARealWorldCharacter::Input_VaultStarted()
{
	TryExecuteVaultOrMantle();
}

void ARealWorldCharacter::SetAimingState(bool bNewAiming)
{
	bIsAiming = bNewAiming;

	if (CombatComponent)
	{
		CombatComponent->SetAiming(bNewAiming);
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (bIsAiming)
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = true;
		CameraBoom->CameraLagSpeed = 22.0f;
	}
	else
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->bUseControllerDesiredRotation = false;
		CameraBoom->CameraLagSpeed = 10.0f;
	}
}

void ARealWorldCharacter::UpdateLocomotionState(float DeltaTime)
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;

	if (bIsVaultInProgress)
	{
		CurrentLocomotionState = ELocomotionState::Vault;
		return;
	}

	if (LandingTimer > 0.0f)
	{
		LandingTimer -= DeltaTime;
		CurrentLocomotionState = ELocomotionState::Land;
		return;
	}

	if (MoveComp->IsFalling())
	{
		CurrentLocomotionState = (MoveComp->Velocity.Z > 0.0f) ? ELocomotionState::Jump : ELocomotionState::Fall;
		return;
	}

	const float Speed = GetVelocity().Size2D();

	if (bIsCrouched)
	{
		CurrentLocomotionState = ELocomotionState::Crouch;
		TargetSpeed = CrouchSpeed;
	}
	else if (Speed < 10.0f)
	{
		CurrentLocomotionState = ELocomotionState::Idle;
		TargetSpeed = 0.0f;
	}
	else if (bWantsToSprint && !bIsExhausted && CurrentStamina > 0.0f && Speed > 150.0f && !bIsAiming)
	{
		CurrentLocomotionState = ELocomotionState::Sprint;
		TargetSpeed = SprintSpeed;
	}
	else
	{
		CurrentLocomotionState = ELocomotionState::Jog;
		TargetSpeed = JogSpeed;
	}

	MoveComp->MaxWalkSpeed = FMath::FInterpTo(MoveComp->MaxWalkSpeed, TargetSpeed > 0.0f ? TargetSpeed : JogSpeed, DeltaTime, SpeedInterpSpeed);
}

void ARealWorldCharacter::UpdateCameraTransform(float DeltaTime)
{
	float TargetArmLength = DefaultArmLength;
	FVector TargetSocketOffset = DefaultSocketOffset;
	float TargetFOVValue = DefaultFOV;

	if (bIsAiming)
	{
		TargetArmLength = AimArmLength;
		TargetSocketOffset = AimSocketOffset;
		TargetFOVValue = AimFOV;
	}
	else if (CurrentLocomotionState == ELocomotionState::Sprint)
	{
		TargetArmLength = SprintArmLength;
		TargetSocketOffset = DefaultSocketOffset;
		TargetFOVValue = SprintFOV;
	}
	else if (bIsCrouched)
	{
		TargetArmLength = DefaultArmLength;
		TargetSocketOffset = CrouchSocketOffset;
		TargetFOVValue = DefaultFOV;
	}

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, 8.0f);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, 8.0f);
	FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOVValue, DeltaTime, 8.0f));
}

void ARealWorldCharacter::UpdateStamina(float DeltaTime)
{
	if (CurrentLocomotionState == ELocomotionState::Sprint)
	{
		TimeSinceLastSprint = 0.0f;
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - (SprintStaminaDrainRate * DeltaTime));

		if (CurrentStamina <= 0.0f)
		{
			bIsExhausted = true;
			bWantsToSprint = false;
		}
	}
	else
	{
		TimeSinceLastSprint += DeltaTime;

		if (TimeSinceLastSprint >= StaminaRecoveryDelay)
		{
			CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (StaminaRecoveryRate * DeltaTime));

			if (bIsExhausted && CurrentStamina >= ExhaustionRecoveryThreshold)
			{
				bIsExhausted = false;
			}
		}
	}
}

void ARealWorldCharacter::UpdateFootIK(float DeltaTime)
{
	if (!GetMesh()) return;

	auto TraceFootAndNormal = [this](FName SocketName, FRotator& OutRotation) -> float
	{
		const FVector SocketLoc = GetMesh()->GetSocketLocation(SocketName);
		const FVector Start = FVector(SocketLoc.X, SocketLoc.Y, GetActorLocation().Z);
		const FVector End = Start - FVector(0.0f, 0.0f, 65.0f + GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			const float GroundZ = Hit.ImpactPoint.Z;
			const float CapsuleBaseZ = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

			// Calculate slope adaptation rotation from impact normal
			const FVector Normal = Hit.ImpactNormal;
			const float Pitch = -FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z));
			const float Roll = FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z));
			OutRotation = FRotator(Pitch, 0.0f, Roll);

			return GroundZ - CapsuleBaseZ;
		}

		OutRotation = FRotator::ZeroRotator;
		return 0.0f;
	};

	FRotator LeftTargetRot;
	FRotator RightTargetRot;
	const float LeftTargetOffset = TraceFootAndNormal(TEXT("foot_l"), LeftTargetRot);
	const float RightTargetOffset = TraceFootAndNormal(TEXT("foot_r"), RightTargetRot);

	// Smoothly interpolate foot heights
	LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, LeftTargetOffset, DeltaTime, 14.0f);
	RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, RightTargetOffset, DeltaTime, 14.0f);

	// Smoothly interpolate foot rotations for slope alignment
	LeftFootRotation = FMath::RInterpTo(LeftFootRotation, LeftTargetRot, DeltaTime, 14.0f);
	RightFootRotation = FMath::RInterpTo(RightFootRotation, RightTargetRot, DeltaTime, 14.0f);

	// Pelvis Z adjustment: drops pelvis when one foot is below the capsule base to avoid leg hyperextension
	const float TargetPelvisOffset = FMath::Min(0.0f, FMath::Min(LeftTargetOffset, RightTargetOffset));
	PelvisOffsetZ = FMath::FInterpTo(PelvisOffsetZ, TargetPelvisOffset, DeltaTime, 12.0f);
}

FVaultResult ARealWorldCharacter::DetectVaultObstacle()
{
	FVaultResult Result;
	Result.bCanVault = false;

	const FVector ActorLoc = GetActorLocation();
	const FVector Forward = GetActorForwardVector();

	const FVector TraceStart = ActorLoc + FVector(0.0f, 0.0f, -20.0f);
	const FVector TraceEnd = TraceStart + (Forward * VaultForwardTraceDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = false;

	FHitResult ForwardHit;
	bool bHitObstacle = GetWorld()->LineTraceSingleByChannel(ForwardHit, TraceStart, TraceEnd, ECC_WorldStatic, Params);
	if (!bHitObstacle) return Result;

	if (ForwardHit.GetActor() && ForwardHit.GetActor()->IsRootComponentMovable())
	{
		return Result;
	}

	const FVector DownStart = ForwardHit.ImpactPoint + (Forward * 25.0f) + FVector(0.0f, 0.0f, MaxVaultHeight + 10.0f);
	const FVector DownEnd = DownStart - FVector(0.0f, 0.0f, MaxVaultHeight + 30.0f);

	FHitResult DownHit;
	bool bHitLedge = GetWorld()->LineTraceSingleByChannel(DownHit, DownStart, DownEnd, ECC_WorldStatic, Params);
	if (!bHitLedge) return Result;

	const float LedgeZ = DownHit.ImpactPoint.Z;
	const float ObstacleHeight = LedgeZ - (ActorLoc.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	if (ObstacleHeight < MinVaultHeight || ObstacleHeight > MaxVaultHeight)
	{
		return Result;
	}

	const FVector LedgeClearanceCenter = DownHit.ImpactPoint + FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 5.0f);
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	bool bClearanceBlocked = GetWorld()->OverlapBlockingTestByChannel(LedgeClearanceCenter, FQuat::Identity, ECC_WorldStatic, CapsuleShape, Params);
	if (bClearanceBlocked) return Result;

	const FVector LandingTarget = DownHit.ImpactPoint + (Forward * 85.0f);
	FVector AdjustedGround;
	if (!ValidateLandingSurface(LandingTarget, AdjustedGround))
	{
		return Result;
	}

	Result.bCanVault = true;
	Result.LedgeLocation = DownHit.ImpactPoint;
	Result.LandingLocation = AdjustedGround + FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0f);
	Result.ObstacleHeight = ObstacleHeight;
	return Result;
}

bool ARealWorldCharacter::ValidateLandingSurface(const FVector& LandingPoint, FVector& OutAdjustedGround)
{
	const FVector TraceStart = LandingPoint + FVector(0.0f, 0.0f, 60.0f);
	const FVector TraceEnd = LandingPoint - FVector(0.0f, 0.0f, 180.0f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
	{
		const float WalkableZ = GetCharacterMovement()->GetWalkableFloorZ();
		if (Hit.ImpactNormal.Z >= WalkableZ)
		{
			OutAdjustedGround = Hit.ImpactPoint;
			return true;
		}
	}

	return false;
}

bool ARealWorldCharacter::TryExecuteVaultOrMantle()
{
	if (bIsVaultInProgress) return false;

	FVaultResult VaultResult = DetectVaultObstacle();
	if (!VaultResult.bCanVault) return false;

	bIsVaultInProgress = true;
	CurrentLocomotionState = ELocomotionState::Vault;

	if (MotionWarpingComponent)
	{
		const FRotator TargetRot = GetActorRotation();
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(FName("LedgeWarpTarget"), VaultResult.LedgeLocation, TargetRot);
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(FName("VaultLandingTarget"), VaultResult.LandingLocation, TargetRot);
	}

	UAnimMontage* MontageToPlay = (VaultResult.ObstacleHeight > 110.0f) ? HighMantleMontage : LowVaultMontage;
	if (MontageToPlay && GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(MontageToPlay);
	}

	VaultStartPos = GetActorLocation();
	VaultEndPos = VaultResult.LandingLocation;
	VaultAlpha = 0.0f;
	VaultDuration = (VaultResult.ObstacleHeight > 110.0f) ? 0.85f : 0.55f;

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	return true;
}

void ARealWorldCharacter::ProcessVaultMovement(float DeltaTime)
{
	VaultAlpha += (DeltaTime / VaultDuration);

	if (VaultAlpha >= 1.0f)
	{
		SetActorLocation(VaultEndPos);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		bIsVaultInProgress = false;
		CurrentLocomotionState = ELocomotionState::Idle;
		return;
	}

	FVector CurrentPos = FMath::Lerp(VaultStartPos, VaultEndPos, VaultAlpha);
	const float ArcHeight = 40.0f;
	CurrentPos.Z += FMath::Sin(VaultAlpha * PI) * ArcHeight;

	SetActorLocation(CurrentPos);
}

float ARealWorldCharacter::GetNormalizedSpeed() const
{
	const float Speed = GetVelocity().Size2D();
	return FMath::Clamp(Speed / SprintSpeed, 0.0f, 1.0f);
}
