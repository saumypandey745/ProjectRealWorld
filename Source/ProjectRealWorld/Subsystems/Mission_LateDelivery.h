#pragma once

#include "CoreMinimal.h"
#include "Subsystems/MissionSubsystem.h"
#include "Mission_LateDelivery.generated.h"

/**
 * Vertical Slice Mission: "Late Delivery"
 * Fully original, 10-stage open-world courier delivery mission.
 */
UCLASS(BlueprintType)
class PROJECTREALWORLD_API UMission_LateDelivery : public UMissionDataAsset
{
	GENERATED_BODY()

public:
	UMission_LateDelivery();
};
