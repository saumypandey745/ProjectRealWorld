#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AI/RealWorldScheduleTypes.h"
#include "RealWorldNPCAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UBehaviorTreeComponent;
class UBlackboardComponent;

UCLASS()
class PROJECTREALWORLD_API ARealWorldNPCAIController : public AAIController
{
	GENERATED_BODY()

public:
	ARealWorldNPCAIController();

	virtual void OnPossess(APawn* InPawn) override;

	/** Handle state changes dispatched from character */
	UFUNCTION(BlueprintCallable, Category = "AI")
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
};
