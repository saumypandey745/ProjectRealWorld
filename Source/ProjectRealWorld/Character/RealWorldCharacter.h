#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Character/RealWorldCharacterCustomization.h"
#include "RealWorldCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class URealWorldInteractionComponent;
class URealWorldInputConfig;
class URealWorldCombatComponent;
class UMotionWarpingComponent;
class UAnimMontage;

UENUM(BlueprintType)
enum class ELocomotionState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Walk UMETA(DisplayName = "Walk"),
	Jog UMETA(DisplayName = "Jog"),
	Sprint UMETA(DisplayName = "Sprint"),
	Crouch UMETA(DisplayName = "Crouch"),
	Jump UMETA(DisplayName = "Jump"),
	Fall UMETA(DisplayName = "Fall"),
	Land UMETA(DisplayName = "Land"),
	Vault UMETA(DisplayName = "Vault"),
	Interact UMETA(DisplayName = "Interact"),
	InVehicle UMETA(DisplayName = "InVehicle")
};

UENUM(BlueprintType)
enum class EUpperBodyState : uint8
{
	Relaxed UMETA(DisplayName = "Relaxed"),
	Aiming UMETA(DisplayName = "Aiming"),
	Firing UMETA(DisplayName = "Firing"),
	Interacting UMETA(DisplayName = "Interacting"),
	UsingPhone UMETA(DisplayName = "UsingPhone")
};

USTRUCT(BlueprintType)
struct FVaultResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bCanVault = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LedgeLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LandingLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ObstacleHeight = 0.0f;
};

UCLASS()
class PROJECTREALWORLD_API ARealWorldCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARealWorldCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;

	// --- Modular Mesh Components (MetaHuman Standard Architecture) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularCharacter")
	TObjectPtr<USkeletalMeshComponent> FaceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularCharacter")
	TObjectPtr<USkeletalMeshComponent> TorsoMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularCharacter")
	TObjectPtr<USkeletalMeshComponent> LegsMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularCharacter")
	TObjectPtr<USkeletalMeshComponent> FeetMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularCharacter")
	TObjectPtr<USkeletalMeshComponent> HairMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularCharacter")
	TObjectPtr<USkeletalMeshComponent> AccessoriesMesh;

	// --- Core Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<URealWorldInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<URealWorldCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|MotionWarping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	// --- Enhanced Input Configuration ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<URealWorldInputConfig> InputConfig;

	// --- Customization Settings ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	FRealWorldCharacterCustomization CustomizationData;

	UFUNCTION(BlueprintCallable, Category = "Customization")
	void ApplyCustomization(const FRealWorldCharacterCustomization& NewData);

	// --- Locomotion Tuning ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Speeds")
	float WalkSpeed = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Speeds")
	float JogSpeed = 480.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Speeds")
	float SprintSpeed = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Speeds")
	float CrouchSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Acceleration")
	float BaseAcceleration = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Acceleration")
	float BaseBrakingDeceleration = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Acceleration")
	float SpeedInterpSpeed = 8.0f;

	// --- State Flags ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion|State")
	ELocomotionState CurrentLocomotionState = ELocomotionState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion|State")
	EUpperBodyState CurrentUpperBodyState = EUpperBodyState::Relaxed;

	// --- Hit Reaction ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	bool bIsHitReacting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	FVector LastHitDirection = FVector::ZeroVector;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyHitReaction(const FVector& HitDirection, float Damage);

	// --- Stamina Architecture ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Config")
	float MaxStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina|State")
	float CurrentStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Config")
	float SprintStaminaDrainRate = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Config")
	float StaminaRecoveryDelay = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Config")
	float StaminaRecoveryRate = 22.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Config")
	float ExhaustionRecoveryThreshold = 25.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina|State")
	bool bIsExhausted = false;

	// --- Camera Tuning ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Tuning")
	float DefaultArmLength = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Tuning")
	float SprintArmLength = 330.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Tuning")
	float AimArmLength = 135.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Tuning")
	FVector DefaultSocketOffset = FVector(0.0f, 55.0f, 45.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Tuning")
	FVector AimSocketOffset = FVector(0.0f, 65.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Tuning")
	FVector CrouchSocketOffset = FVector(0.0f, 45.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Tuning")
	float DefaultFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Tuning")
	float SprintFOV = 98.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Tuning")
	float AimFOV = 65.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Tuning")
	float CameraSensitivityX = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Tuning")
	float CameraSensitivityY = 1.0f;

	// --- Vault / Mantle Tuning ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Tuning")
	float MinVaultHeight = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Tuning")
	float MaxVaultHeight = 210.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Tuning")
	float VaultForwardTraceDistance = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Montages")
	TObjectPtr<UAnimMontage> LowVaultMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Montages")
	TObjectPtr<UAnimMontage> HighMantleMontage;

	// --- Complete Foot IK Outputs (Control Rig & AnimGraph) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|FootIK")
	float LeftFootIKOffset = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|FootIK")
	float RightFootIKOffset = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|FootIK")
	FRotator LeftFootRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|FootIK")
	FRotator RightFootRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|FootIK")
	float PelvisOffsetZ = 0.0f;

	// --- Vitals ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vitals")
	float CurrentHealth = 100.0f;

	UFUNCTION(BlueprintCallable, Category = "Vault")
	bool TryExecuteVaultOrMantle();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetAimingState(bool bNewAiming);

	UFUNCTION(BlueprintPure, Category = "Locomotion")
	float GetNormalizedSpeed() const;

protected:
	virtual void BeginPlay() override;

	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_LookMouse(const FInputActionValue& Value);
	void Input_JumpStarted();
	void Input_JumpStopped();
	void Input_SprintStarted();
	void Input_SprintStopped();
	void Input_CrouchToggle();
	void Input_Interact();
	void Input_AttackStarted();
	void Input_AimStarted();
	void Input_AimStopped();
	void Input_ReloadStarted();
	void Input_VaultStarted();

	void UpdateLocomotionState(float DeltaTime);
	void UpdateCameraTransform(float DeltaTime);
	void UpdateStamina(float DeltaTime);
	void UpdateFootIK(float DeltaTime);
	void ProcessVaultMovement(float DeltaTime);

	FVaultResult DetectVaultObstacle();
	bool ValidateLandingSurface(const FVector& LandingPoint, FVector& OutAdjustedGround);

private:
	bool bWantsToSprint = false;
	bool bIsAiming = false;
	bool bIsVaultInProgress = false;
	float TimeSinceLastSprint = 0.0f;
	float LandingTimer = 0.0f;
	float LandingDuration = 0.22f;
	float HitReactionTimer = 0.0f;

	FVector VaultStartPos = FVector::ZeroVector;
	FVector VaultEndPos = FVector::ZeroVector;
	float VaultAlpha = 0.0f;
	float VaultDuration = 0.65f;
	float TargetSpeed = 0.0f;
};
