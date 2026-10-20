#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "MissionSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMissionType : uint8
{
	MainStory UMETA(DisplayName = "Main Story"),
	SideActivity UMETA(DisplayName = "Side Activity"),
	DynamicEncounter UMETA(DisplayName = "Dynamic Encounter")
};

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
	GoToLocation UMETA(DisplayName = "Go To Location"),
	TalkToNPC UMETA(DisplayName = "Talk To NPC"),
	EliminateTarget UMETA(DisplayName = "Eliminate Target"),
	DeliverVehicle UMETA(DisplayName = "Deliver Vehicle"),
	SurviveTimer UMETA(DisplayName = "Survive Timer")
};

USTRUCT(BlueprintType)
struct FMissionObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	EObjectiveType Type = EObjectiveType::GoToLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	float AcceptanceRadius = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName TargetActorTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Objective")
	bool bIsCompleted = false;
};

UCLASS(BlueprintType)
class PROJECTREALWORLD_API UMissionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FName MissionID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FText Synopsis;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	EMissionType Category = EMissionType::MainStory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	TArray<FMissionObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Rewards")
	float CashReward = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Rewards")
	int32 ReputationReward = 50;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStarted, UMissionDataAsset*, Mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionObjectiveUpdated, UMissionDataAsset*, Mission, int32, ObjectiveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionCompleted, UMissionDataAsset*, Mission, bool, bSuccess);

UCLASS()
class PROJECTREALWORLD_API UMissionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Start a data-driven mission */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	bool StartMission(UMissionDataAsset* MissionAsset);

	/** Progress current objective */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void AdvanceObjective();

	/** Fail current mission */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void FailMission(const FText& Reason);

	/** Complete current mission and disburse rewards */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void CompleteMission();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Active")
	TObjectPtr<UMissionDataAsset> ActiveMission;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Active")
	int32 CurrentObjectiveIndex = 0;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Mission|Events")
	FOnMissionStarted OnMissionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Mission|Events")
	FOnMissionObjectiveUpdated OnMissionObjectiveUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Mission|Events")
	FOnMissionCompleted OnMissionCompleted;

private:
	UPROPERTY()
	TArray<FName> CompletedMissionIDs;
};
