#pragma once

#include "CoreMinimal.h"
#include "RealWorldScheduleTypes.generated.h"

UENUM(BlueprintType)
enum class ENPCState : uint8
{
	IDLE UMETA(DisplayName = "IDLE"),
	WALKING UMETA(DisplayName = "WALKING"),
	WORKING UMETA(DisplayName = "WORKING"),
	SHOPPING UMETA(DisplayName = "SHOPPING"),
	EATING UMETA(DisplayName = "EATING"),
	DRIVING UMETA(DisplayName = "DRIVING"),
	SOCIALIZING UMETA(DisplayName = "SOCIALIZING"),
	GOING_HOME UMETA(DisplayName = "GOING_HOME"),
	FLEEING UMETA(DisplayName = "FLEEING"),
	FIGHTING UMETA(DisplayName = "FIGHTING"),
	CALLING_POLICE UMETA(DisplayName = "CALLING_POLICE"),
	INJURED UMETA(DisplayName = "INJURED"),
	DEAD UMETA(DisplayName = "DEAD")
};

UENUM(BlueprintType)
enum class ENPCSimulationLOD : uint8
{
	FullTick UMETA(DisplayName = "Full Tick (Near)"),
	ReducedTick UMETA(DisplayName = "Reduced Tick (Medium)"),
	FarBudgeted UMETA(DisplayName = "Far Budgeted (Far)"),
	Dormant UMETA(DisplayName = "Dormant (Background)")
};

USTRUCT(BlueprintType)
struct FNPCDailyScheduleEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	float StartHour = 8.0f; // 24h clock

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	ENPCState AssignedState = ENPCState::WORKING;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	FName LocationTag;
};
