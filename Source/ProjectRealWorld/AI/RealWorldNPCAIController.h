#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AI/RealWorldScheduleTypes.h"
#include "RealWorldNPCAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class ARealWorldNPCCharacter;

UCLASS()
class PROJECTREALWORLD_API ARealWorldNPCAIController : public AAIController
{
	GENERATED_BODY()

public:
	ARealWorldNPCAIController();

	virtual void OnPossess(APawn* InPawn) override;

	/** Handle state changes dispatched from character */
	UFUNCTION()
	void OnNPCStateChanged(ENPCState NewState);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

protected:
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	UFUNCTION()
	void OnTargetPerceptionInfoUpdated(const FActorPerceptionUpdateInfo& UpdateInfo);

private:
	void SetupPerception();

	/** Move NPC to a random navmesh point within Radius cm */
	void MoveToRandomNavMeshPoint(ARealWorldNPCCharacter* NPC, float Radius);

	/** Sprint NPC away from their stored LastDangerLocation */
	void FleeFromDanger(ARealWorldNPCCharacter* NPC);
};
