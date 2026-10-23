#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI/RealWorldScheduleTypes.h"
#include "AI/RealWorldCrowdTypes.h"
#include "Character/RealWorldFacialAnimationTypes.h"
#include "Interaction/InteractableInterface.h"
#include "RealWorldNPCCharacter.generated.h"

class USkeletalMeshComponent;

// Delegate so AI controller can bind to state changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNPCStateChanged, ENPCState, NewState);

UCLASS()
class PROJECTREALWORLD_API ARealWorldNPCCharacter : public ACharacter, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ARealWorldNPCCharacter();

	virtual void Tick(float DeltaTime) override;

	// --- Modular Skeletal Meshes (MetaHuman Standard Assembly) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularAppearance")
	TObjectPtr<USkeletalMeshComponent> HeadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularAppearance")
	TObjectPtr<USkeletalMeshComponent> TorsoMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularAppearance")
	TObjectPtr<USkeletalMeshComponent> LegsMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularAppearance")
	TObjectPtr<USkeletalMeshComponent> FeetMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularAppearance")
	TObjectPtr<USkeletalMeshComponent> HairMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ModularAppearance")
	TObjectPtr<USkeletalMeshComponent> AccessoriesMesh;

	// --- Archetype & Identity ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Archetype")
	ENPCArchetype Archetype = ENPCArchetype::CivilianMale;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
	ENPCState CurrentState = ENPCState::IDLE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
	ENPCSimulationLOD CurrentSimulationLOD = ENPCSimulationLOD::FullTick;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Facial")
	EFacialEmotionState CurrentEmotion = EFacialEmotionState::Neutral;

	// --- Store Interaction Substate Machine ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Store")
	EStoreInteractionSubstate StoreSubstate = EStoreInteractionSubstate::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
	TArray<FNPCDailyScheduleEntry> DailySchedule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Vitals")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
	FText NPCName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
	FText Occupation;

	UFUNCTION(BlueprintCallable, Category = "NPC|Archetype")
	void SetArchetype(ENPCArchetype NewArchetype);

	UFUNCTION(BlueprintCallable, Category = "NPC|Facial")
	void SetFacialEmotion(EFacialEmotionState NewEmotion);

	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void SetNPCState(ENPCState NewState);

	// --- AI Controller Delegate ---
	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnNPCStateChanged OnNPCStateChangedDelegate;

	// --- Navigation Data (written by EvaluateDailySchedule) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Navigation")
	FVector HomeLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Navigation")
	FVector CurrentScheduleDestination = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Reactions")
	FVector LastDangerLocation = FVector::ZeroVector;

	UFUNCTION(BlueprintCallable, Category = "NPC|Reactions")
	void ReactToDanger(FVector DangerSource);

	UFUNCTION(BlueprintCallable, Category = "NPC|Reactions")
	void ReactToWeather(float RainIntensity);

	UFUNCTION(BlueprintCallable, Category = "NPC|Reactions")
	void ReactToSprintProximity(const FVector& SprinterLocation);

	UFUNCTION(BlueprintCallable, Category = "NPC|Reactions")
	void ReactToCollision(AActor* CollidingActor, const FVector& HitNormal);

	UFUNCTION(BlueprintCallable, Category = "NPC|Reactions")
	void ReactToWeaponDischarge(const FVector& GunshotLocation);

	UFUNCTION(BlueprintCallable, Category = "NPC|Reactions")
	void ReactToVehicleAccident(const FVector& CrashLocation);

	UFUNCTION(BlueprintCallable, Category = "NPC|Store")
	void StartStoreVisitRoutine(const FVector& StoreEntrance, const FVector& ShelfA, const FVector& Counter);

	// IInteractableInterface
	virtual FText GetInteractionPrompt_Implementation(ARealWorldCharacter* InstigatorCharacter) override;
	virtual bool CanInteract_Implementation(ARealWorldCharacter* InstigatorCharacter) override;
	virtual void Interact_Implementation(ARealWorldCharacter* InstigatorCharacter) override;

protected:
	virtual void BeginPlay() override;

	void UpdateSimulationLOD();
	void EvaluateDailySchedule(float CurrentHour);
	void UpdateStoreShoppingBehavior(float DeltaTime);

private:
	float DistanceToPlayer = 0.0f;
	float ScheduleCheckTimer = 0.0f;
	float StoreActionTimer = 0.0f;

	FVector TargetStoreEntrance = FVector::ZeroVector;
	FVector TargetShelf = FVector::ZeroVector;
	FVector TargetCounter = FVector::ZeroVector;
};
