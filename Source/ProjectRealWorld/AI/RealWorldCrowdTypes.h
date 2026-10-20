#pragma once

#include "CoreMinimal.h"
#include "RealWorldCrowdTypes.generated.h"

UENUM(BlueprintType)
enum class ENPCArchetype : uint8
{
	CivilianMale UMETA(DisplayName = "Civilian Male"),
	CivilianFemale UMETA(DisplayName = "Civilian Female"),
	Business UMETA(DisplayName = "Business Executive"),
	Casual UMETA(DisplayName = "Casual Youth"),
	ShopWorker UMETA(DisplayName = "Shop Worker"),
	Security UMETA(DisplayName = "Security Guard"),
	MunicipalWorker UMETA(DisplayName = "Municipal Worker"),
	Police UMETA(DisplayName = "Police Officer")
};

UENUM(BlueprintType)
enum class EStoreInteractionSubstate : uint8
{
	None UMETA(DisplayName = "None"),
	ApproachStore UMETA(DisplayName = "Approach Store"),
	OpenDoor UMETA(DisplayName = "Open Door"),
	EnterStore UMETA(DisplayName = "Enter Store"),
	BrowseAisles UMETA(DisplayName = "Browse Aisles"),
	InspectItem UMETA(DisplayName = "Inspect Item"),
	ApproachCounter UMETA(DisplayName = "Approach Counter"),
	Checkout UMETA(DisplayName = "Checkout"),
	ExitStore UMETA(DisplayName = "Exit Store")
};

USTRUCT(BlueprintType)
struct FRealWorldCrowdSpawnPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
	ENPCArchetype PreferredArchetype = ENPCArchetype::CivilianMale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
	bool bStartSeatedOnBench = false;
};
