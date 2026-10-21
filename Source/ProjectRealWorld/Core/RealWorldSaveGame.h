#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RealWorldSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FOwnedVehicleSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FTransform LastTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float FuelPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float DamagePercent = 0.0f;
};

USTRUCT(BlueprintType)
struct FMissionSaveState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FName MissionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 CurrentObjectiveIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	bool bIsCompleted = false;
};

UCLASS()
class PROJECTREALWORLD_API URealWorldSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	URealWorldSaveGame();

	static const int32 CURRENT_SAVE_VERSION = 2;

	/** Version flag for backwards-compatible migrations */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveData")
	int32 SaveVersion = CURRENT_SAVE_VERSION;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FString SaveSlotName = TEXT("Slot_01");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FDateTime Timestamp;

	// --- Player State ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|Player")
	FTransform PlayerTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|Player")
	float PlayerHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|Player")
	float PlayerStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|Player")
	float Cash = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|Player")
	float BankBalance = 8500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|Player")
	int32 WantedLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|Player")
	TArray<FName> InventoryItemIDs;

	// --- Vehicles ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|World")
	TArray<FOwnedVehicleSaveData> OwnedVehicles;

	// --- Missions ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|World")
	TArray<FMissionSaveState> MissionStates;

	// --- World State ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|World")
	float TimeOfDayHours = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|World")
	uint8 CurrentWeatherType = 0;

	// --- Migration and Corruption Validation ---
	UFUNCTION(BlueprintCallable, Category = "SaveData|Validation")
	bool ValidateAndMigrateSaveData();

	UFUNCTION(BlueprintCallable, Category = "SaveData|Validation")
	void ResetToSafeDefaults();
};
