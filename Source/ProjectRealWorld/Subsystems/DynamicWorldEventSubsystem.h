#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DynamicWorldEventSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDynamicEventType : uint8
{
	TrafficAccident UMETA(DisplayName = "Traffic Accident"),
	PedestrianArgument UMETA(DisplayName = "Pedestrian Argument"),
	MinorStreetFight UMETA(DisplayName = "Minor Street Fight"),
	VehicleBreakdown UMETA(DisplayName = "Vehicle Breakdown"),
	ShopDisturbance UMETA(DisplayName = "Shop Disturbance"),
	PoliceResponse UMETA(DisplayName = "Police Response Encounter"),
	WeatherDisruption UMETA(DisplayName = "Weather Disruption")
};

USTRUCT(BlueprintType)
struct FDynamicWorldEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
	FGuid EventID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
	EDynamicEventType Type = EDynamicEventType::TrafficAccident;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
	float Radius = 1500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
	float DurationRemaining = 120.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
	FText Synopsis;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
	bool bIsActive = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDynamicEventTriggered, const FDynamicWorldEvent&, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDynamicEventResolved, FGuid, EventID);

UCLASS()
class PROJECTREALWORLD_API UDynamicWorldEventSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Trigger a custom dynamic event at target location */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents")
	bool TriggerEvent(EDynamicEventType Type, FVector TargetLocation);

	/** Force clean resolution of active event */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents")
	void ResolveEvent(FGuid EventID);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WorldEvents")
	TArray<FDynamicWorldEvent> ActiveEvents;

	UPROPERTY(BlueprintAssignable, Category = "WorldEvents|Events")
	FOnDynamicEventTriggered OnDynamicEventTriggered;

	UPROPERTY(BlueprintAssignable, Category = "WorldEvents|Events")
	FOnDynamicEventResolved OnDynamicEventResolved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldEvents|Config")
	float MinEventIntervalSeconds = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldEvents|Config")
	int32 MaxConcurrentEvents = 2;

protected:
	void EvaluateAmbientEventSpawning(float DeltaTime);
	EDynamicEventType SelectContextualEventType(bool bIsNight, float RainIntensity) const;

private:
	float CooldownTimer = 60.0f; // Initial 1 min breathing room
	float EvaluationTimer = 0.0f;
};
