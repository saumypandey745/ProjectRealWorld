#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RealWorldInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractableTargetChanged, AActor*, NewTarget, const FText&, PromptText);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREALWORLD_API URealWorldInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URealWorldInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Trigger interaction with current focused actor */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool PerformInteraction();

	/** Maximum distance to detect interactive objects (in centimeters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Settings")
	float MaxInteractionDistance = 280.0f;

	/** Trace radius for sphere sweep */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Settings")
	float TraceRadius = 35.0f;

	/** Delegate fired when focused interactable changes (for HUD prompts) */
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractableTargetChanged OnInteractableTargetChanged;

	/** Get current focused actor */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentFocusedActor() const { return FocusedActor.Get(); }

protected:
	virtual void BeginPlay() override;

	void UpdateInteractionTrace();
	bool ValidateLineOfSight(AActor* TargetActor, const FVector& EyeLocation, const FVector& TargetLocation) const;

private:
	TWeakObjectPtr<AActor> FocusedActor;
	FText CachedPrompt;
};
