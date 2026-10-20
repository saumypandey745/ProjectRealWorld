#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "RealWorldPlayerStart.generated.h"

UCLASS()
class PROJECTREALWORLD_API ARealWorldPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	ARealWorldPlayerStart(const FObjectInitializer& ObjectInitializer);

	/** District / Neighborhood identifier where this spawn point belongs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	FName DistrictTag = TEXT("Downtown");

	/** Desired camera look yaw upon initial player possession */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	float InitialCameraYaw = 0.0f;

	/** Verify that spawn location is unobstructed by physical geometry */
	UFUNCTION(BlueprintPure, Category = "SpawnPoint")
	bool ValidateSpawnClearance() const;
};
