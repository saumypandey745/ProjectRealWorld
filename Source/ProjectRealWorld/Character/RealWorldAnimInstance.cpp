#include "Character/RealWorldAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void URealWorldAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningCharacter = Cast<ARealWorldCharacter>(TryGetPawnOwner());
	PreviousSpeed = 0.0f;
}

void URealWorldAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter)
	{
		OwningCharacter = Cast<ARealWorldCharacter>(TryGetPawnOwner());
		if (!OwningCharacter) return;
	}

	UCharacterMovementComponent* MoveComp = OwningCharacter->GetCharacterMovement();
	if (!MoveComp) return;

	Velocity = OwningCharacter->GetVelocity();
	Speed = Velocity.Size2D();
	Acceleration = MoveComp->GetCurrentAcceleration();

	const FRotator ActorRot = OwningCharacter->GetActorRotation();
	Direction = CalculateDirection(Velocity, ActorRot);

	// Start / Stop transition detection for responsive procedural locomotion
	const float AccelSize = Acceleration.Size2D();
	bIsStarting = (AccelSize > 100.0f && Speed < 120.0f && PreviousSpeed < 30.0f);
	bIsStopping = (AccelSize < 50.0f && Speed > 40.0f && Speed < PreviousSpeed);
	StrideWarpingWeight = FMath::Clamp(Speed / FMath::Max(1.0f, OwningCharacter->JogSpeed), 0.5f, 1.4f);

	bIsFalling = MoveComp->IsFalling();
	bIsCrouched = OwningCharacter->bIsCrouched;
	bIsSprinting = (OwningCharacter->CurrentLocomotionState == ELocomotionState::Sprint);
	bIsAiming = (OwningCharacter->CurrentUpperBodyState == EUpperBodyState::Aiming);
	bIsVaulting = (OwningCharacter->CurrentLocomotionState == ELocomotionState::Vault);
	bIsLanding = (OwningCharacter->CurrentLocomotionState == ELocomotionState::Land);
	bIsExhausted = OwningCharacter->bIsExhausted;

	// Hit Reaction Telemetry
	bIsHitReacting = OwningCharacter->bIsHitReacting;
	HitReactDirection = OwningCharacter->LastHitDirection;

	LocomotionState = OwningCharacter->CurrentLocomotionState;
	UpperBodyState = OwningCharacter->CurrentUpperBodyState;

	// Complete Foot IK
	LeftFootOffset = OwningCharacter->LeftFootIKOffset;
	RightFootOffset = OwningCharacter->RightFootIKOffset;
	LeftFootRotation = OwningCharacter->LeftFootRotation;
	RightFootRotation = OwningCharacter->RightFootRotation;
	PelvisOffsetZ = OwningCharacter->PelvisOffsetZ;

	// Aim offset for weapon hold
	const FRotator AimRot = OwningCharacter->GetBaseAimRotation();
	const FRotator DeltaAim = UKismetMathLibrary::NormalizedDeltaRotator(AimRot, ActorRot);
	AimYaw = DeltaAim.Yaw;
	AimPitch = DeltaAim.Pitch;

	PreviousSpeed = Speed;
}
