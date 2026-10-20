#include "Subsystems/DynamicWorldEventSubsystem.h"
#include "Subsystems/TimeOfDaySubsystem.h"
#include "Subsystems/WeatherSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UDynamicWorldEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CooldownTimer = 90.0f;
	ActiveEvents.Empty();
}

void UDynamicWorldEventSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Tick and expire active events
	for (int32 i = ActiveEvents.Num() - 1; i >= 0; --i)
	{
		ActiveEvents[i].DurationRemaining -= DeltaTime;
		if (ActiveEvents[i].DurationRemaining <= 0.0f)
		{
			FGuid ExpiredID = ActiveEvents[i].EventID;
			ActiveEvents.RemoveAt(i);
			OnDynamicEventResolved.Broadcast(ExpiredID);
		}
	}

	EvaluateAmbientEventSpawning(DeltaTime);
}

TStatId UDynamicWorldEventSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UDynamicWorldEventSubsystem, STATGROUP_Tickables);
}

bool UDynamicWorldEventSubsystem::TriggerEvent(EDynamicEventType Type, FVector TargetLocation)
{
	if (ActiveEvents.Num() >= MaxConcurrentEvents)
	{
		return false;
	}

	FDynamicWorldEvent NewEvent;
	NewEvent.EventID = FGuid::NewGuid();
	NewEvent.Type = Type;
	NewEvent.Location = TargetLocation;
	NewEvent.DurationRemaining = 120.0f;
	NewEvent.bIsActive = true;

	switch (Type)
	{
	case EDynamicEventType::TrafficAccident:
		NewEvent.Radius = 2500.0f;
		NewEvent.DurationRemaining = 180.0f;
		NewEvent.Synopsis = FText::FromString(TEXT("Minor two-car collision causing traffic delay."));
		break;
	case EDynamicEventType::PedestrianArgument:
		NewEvent.Radius = 1200.0f;
		NewEvent.DurationRemaining = 90.0f;
		NewEvent.Synopsis = FText::FromString(TEXT("Heated verbal dispute between sidewalk pedestrians."));
		break;
	case EDynamicEventType::MinorStreetFight:
		NewEvent.Radius = 1500.0f;
		NewEvent.DurationRemaining = 100.0f;
		NewEvent.Synopsis = FText::FromString(TEXT("Physical brawl drawing a small spectator crowd."));
		break;
	case EDynamicEventType::VehicleBreakdown:
		NewEvent.Radius = 1800.0f;
		NewEvent.DurationRemaining = 240.0f;
		NewEvent.Synopsis = FText::FromString(TEXT("Stalled vehicle with hazards flashing at curbside."));
		break;
	case EDynamicEventType::ShopDisturbance:
		NewEvent.Radius = 1400.0f;
		NewEvent.DurationRemaining = 120.0f;
		NewEvent.Synopsis = FText::FromString(TEXT("Verbal altercation inside commercial store premises."));
		break;
	case EDynamicEventType::PoliceResponse:
		NewEvent.Radius = 3000.0f;
		NewEvent.DurationRemaining = 150.0f;
		NewEvent.Synopsis = FText::FromString(TEXT("Police cruiser responding with sirens to local incident."));
		break;
	case EDynamicEventType::WeatherDisruption:
		NewEvent.Radius = 4000.0f;
		NewEvent.DurationRemaining = 300.0f;
		NewEvent.Synopsis = FText::FromString(TEXT("Fallen tree branch and slick roads slowing district movement."));
		break;
	default:
		break;
	}

	ActiveEvents.Add(NewEvent);
	CooldownTimer = MinEventIntervalSeconds;
	OnDynamicEventTriggered.Broadcast(NewEvent);
	return true;
}

void UDynamicWorldEventSubsystem::ResolveEvent(FGuid EventID)
{
	for (int32 i = 0; i < ActiveEvents.Num(); ++i)
	{
		if (ActiveEvents[i].EventID == EventID)
		{
			ActiveEvents.RemoveAt(i);
			OnDynamicEventResolved.Broadcast(EventID);
			break;
		}
	}
}

void UDynamicWorldEventSubsystem::EvaluateAmbientEventSpawning(float DeltaTime)
{
	CooldownTimer -= DeltaTime;
	if (CooldownTimer > 0.0f || ActiveEvents.Num() >= MaxConcurrentEvents)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	FVector PlayerPos = PlayerPawn->GetActorLocation();

	// Sample context
	bool bIsNight = false;
	float Rain = 0.0f;

	if (UWorld* World = GetWorld())
	{
		if (UTimeOfDaySubsystem* TimeSub = World->GetSubsystem<UTimeOfDaySubsystem>())
		{
			bIsNight = TimeSub->bIsNightTime;
		}
		if (UWeatherSubsystem* WeatherSub = World->GetSubsystem<UWeatherSubsystem>())
		{
			Rain = WeatherSub->RainIntensity;
		}
	}

	// 25% chance per evaluation check to generate an ambient event
	if (FMath::FRand() < 0.25f)
	{
		// Spawn spatial location around 45m - 75m away from player
		float SpawnDistance = FMath::FRandRange(4500.0f, 7500.0f);
		float RandomAngle = FMath::FRandRange(0.0f, 2.0f * PI);
		FVector Offset = FVector(FMath::Cos(RandomAngle) * SpawnDistance, FMath::Sin(RandomAngle) * SpawnDistance, 0.0f);
		FVector EventLocation = PlayerPos + Offset;

		EDynamicEventType ChosenType = SelectContextualEventType(bIsNight, Rain);
		TriggerEvent(ChosenType, EventLocation);
	}
	else
	{
		// Recheck after 30s
		CooldownTimer = 30.0f;
	}
}

EDynamicEventType UDynamicWorldEventSubsystem::SelectContextualEventType(bool bIsNight, float RainIntensity) const
{
	if (RainIntensity > 0.5f && FMath::FRand() < 0.6f)
	{
		return (FMath::FRand() < 0.5f) ? EDynamicEventType::TrafficAccident : EDynamicEventType::WeatherDisruption;
	}

	if (bIsNight)
	{
		float R = FMath::FRand();
		if (R < 0.35f) return EDynamicEventType::MinorStreetFight;
		if (R < 0.70f) return EDynamicEventType::ShopDisturbance;
		return EDynamicEventType::PoliceResponse;
	}

	// Daytime ambient mix
	float R = FMath::FRand();
	if (R < 0.30f) return EDynamicEventType::PedestrianArgument;
	if (R < 0.60f) return EDynamicEventType::VehicleBreakdown;
	if (R < 0.85f) return EDynamicEventType::TrafficAccident;
	return EDynamicEventType::ShopDisturbance;
}
