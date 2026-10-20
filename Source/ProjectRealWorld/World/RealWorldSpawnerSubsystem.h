#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RealWorldSpawnerSubsystem.generated.h"

class ARealWorldNPCCharacter;
class ARealWorldVehicleBase;

UENUM(BlueprintType)
enum class EWorldDistrictType : uint8
{
	DowntownCity UMETA(DisplayName = "Downtown City"),
	Suburbs UMETA(DisplayName = "Suburbs"),
	RuralHighways UMETA(DisplayName = "Rural Highways"),
	IndustrialDocks UMETA(DisplayName = "Industrial Docks"),
	ForestPark UMETA(DisplayName = "Forest & Parklands"),
	UndergroundTransit UMETA(DisplayName = "Underground Transit")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDistrictChanged, EWorldDistrictType, NewDistrict);

UCLASS()
class PROJECTREALWORLD_API URealWorldSpawnerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Current district calculated from player position */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Districts")
	EWorldDistrictType CurrentDistrict = EWorldDistrictType::DowntownCity;

	/** Maximum active ambient civilian NPCs allowed simultaneously */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Budgets")
	int32 MaxActiveNPCs = 40;

	/** Maximum active ambient traffic vehicles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Budgets")
	int32 MaxActiveVehicles = 25;

	/** Minimum distance from player to spawn new actors (prevents pop-in in direct view) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Budgets")
	float MinSpawnDistance = 3500.0f; // 35 meters

	/** Maximum active radius; actors beyond this are culled/pooled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Budgets")
	float MaxActiveRadius = 16000.0f; // 160 meters

	UPROPERTY(BlueprintAssignable, Category = "World|Events")
	FOnDistrictChanged OnDistrictChanged;

	/** Register active interior entry */
	UFUNCTION(BlueprintCallable, Category = "World|Interiors")
	void NotifyInteriorEntered(FName InteriorName);

	UFUNCTION(BlueprintCallable, Category = "World|Interiors")
	void NotifyInteriorExited(FName InteriorName);

protected:
	void UpdateDistrictEvaluation(const FVector& PlayerPos);
	void ManageStreamingBudgets(const FVector& PlayerPos);

private:
	float BudgetCheckTimer = 0.0f;
};
