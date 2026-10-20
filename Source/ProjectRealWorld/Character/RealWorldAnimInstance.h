#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/RealWorldCharacter.h"
#include "RealWorldAnimInstance.generated.h"

/**
 * Production animation data pipeline for Project RealWorld.
 * Exposes locomotion blend space coordinates, start/stop transitions, aim offsets,
 * Control Rig Foot IK (offsets + rotation + pelvis adjustment), and hit reactions.
 */
UCLASS()
class PROJECTREALWORLD_API URealWorldAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// --- Locomotion Blend Space Coordinates ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	float Speed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	float Direction = 0.0f; // [-180, 180] for 8-way directional strafing

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FVector Acceleration = FVector::ZeroVector;

	// --- Start / Stop Dynamic Transition Telemetry ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion|Transitions")
	bool bIsStarting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion|Transitions")
	bool bIsStopping = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion|Transitions")
	float StrideWarpingWeight = 1.0f;

	// --- Discrete Locomotion States ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
	bool bIsFalling = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
	bool bIsCrouched = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
	bool bIsSprinting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
	bool bIsAiming = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
	bool bIsVaulting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
	bool bIsLanding = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
	bool bIsExhausted = false;

	// --- Hit Reaction ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsHitReacting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	FVector HitReactDirection = FVector::ZeroVector;

	// --- Aim Offsets (Upper Body Weapon Blending) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AimOffset")
	float AimYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AimOffset")
	float AimPitch = 0.0f;

	// --- State Enums ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
	ELocomotionState LocomotionState = ELocomotionState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
	EUpperBodyState UpperBodyState = EUpperBodyState::Relaxed;

	// --- Complete Control Rig Foot IK Pipeline ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FootIK")
	float LeftFootOffset = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FootIK")
	float RightFootOffset = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FootIK")
	FRotator LeftFootRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FootIK")
	FRotator RightFootRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FootIK")
	float PelvisOffsetZ = 0.0f;

private:
	UPROPERTY()
	TObjectPtr<ARealWorldCharacter> OwningCharacter;

	float PreviousSpeed = 0.0f;
};
