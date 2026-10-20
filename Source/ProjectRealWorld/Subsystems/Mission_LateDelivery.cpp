#include "Subsystems/Mission_LateDelivery.h"

UMission_LateDelivery::UMission_LateDelivery()
{
	MissionID = FName(TEXT("MIS_LateDelivery"));
	Title = FText::FromString(TEXT("Late Delivery"));
	Synopsis = FText::FromString(TEXT("Transport an urgent priority courier container from the Midtown Commercial District to Harbor Terminal 4 before dock departure."));
	Category = EMissionType::MainStory;
	CashReward = 2500.0f;
	ReputationReward = 75;

	Objectives.Empty();

	// Stage 1: Receive phone/message
	{
		FMissionObjective Obj;
		Obj.Description = FText::FromString(TEXT("Check incoming courier dispatch instructions on your device"));
		Obj.Type = EObjectiveType::GoToLocation;
		Obj.TargetLocation = FVector(0.0f, 0.0f, 100.0f);
		Obj.AcceptanceRadius = 400.0f;
		Objectives.Add(Obj);
	}

	// Stage 2: Travel to starting location
	{
		FMissionObjective Obj;
		Obj.Description = FText::FromString(TEXT("Head to the logistics staging depot at North Point"));
		Obj.Type = EObjectiveType::GoToLocation;
		Obj.TargetLocation = FVector(2500.0f, 1200.0f, 0.0f);
		Obj.AcceptanceRadius = 450.0f;
		Objectives.Add(Obj);
	}

	// Stage 3: Enter vehicle
	{
		FMissionObjective Obj;
		Obj.Description = FText::FromString(TEXT("Enter the assigned transport vehicle at the depot bay"));
		Obj.Type = EObjectiveType::DeliverVehicle;
		Obj.TargetLocation = FVector(2800.0f, 1300.0f, 0.0f);
		Obj.AcceptanceRadius = 350.0f;
		Objectives.Add(Obj);
	}

	// Stage 4: Drive to commercial district
	{
		FMissionObjective Obj;
		Obj.Description = FText::FromString(TEXT("Drive to the commercial retail district"));
		Obj.Type = EObjectiveType::GoToLocation;
		Obj.TargetLocation = FVector(8500.0f, -4200.0f, 0.0f);
		Obj.AcceptanceRadius = 800.0f;
		Objectives.Add(Obj);
	}

	// Stage 5: Pick up package
	{
		FMissionObjective Obj;
		Obj.Description = FText::FromString(TEXT("Collect the sealed cargo container from the store loading dock"));
		Obj.Type = EObjectiveType::GoToLocation;
		Obj.TargetLocation = FVector(8700.0f, -4100.0f, 20.0f);
		Obj.AcceptanceRadius = 250.0f;
		Objectives.Add(Obj);
	}

	// Stage 6: Avoid or respond to unexpected street event
	{
		FMissionObjective Obj;
		Obj.Description = FText::FromString(TEXT("Navigate around the street obstruction on 4th Avenue"));
		Obj.Type = EObjectiveType::GoToLocation;
		Obj.TargetLocation = FVector(5200.0f, -1500.0f, 0.0f);
		Obj.AcceptanceRadius = 600.0f;
		Objectives.Add(Obj);
	}

	// Stage 7: Deliver package to Harbor Gate
	{
		FMissionObjective Obj;
		Obj.Description = FText::FromString(TEXT("Deliver the cargo container to Harbor Gate 3"));
		Obj.Type = EObjectiveType::GoToLocation;
		Obj.TargetLocation = FVector(-7500.0f, 9200.0f, 0.0f);
		Obj.AcceptanceRadius = 500.0f;
		Objectives.Add(Obj);
	}

	// Stage 8: Confirm delivery with receiving agent
	{
		FMissionObjective Obj;
		Obj.Description = FText::FromString(TEXT("Confirm receipt with Harbor Logistics Receiving"));
		Obj.Type = EObjectiveType::TalkToNPC;
		Obj.TargetLocation = FVector(-7650.0f, 9350.0f, 20.0f);
		Obj.AcceptanceRadius = 250.0f;
		Objectives.Add(Obj);
	}

	// Stage 9: Receive payment
	{
		FMissionObjective Obj;
		Obj.Description = FText::FromString(TEXT("Verify wire payment transfer of $2,500"));
		Obj.Type = EObjectiveType::GoToLocation;
		Obj.TargetLocation = FVector(-7650.0f, 9350.0f, 20.0f);
		Obj.AcceptanceRadius = 500.0f;
		Objectives.Add(Obj);
	}

	// Stage 10: Complete and archive mission state
	{
		FMissionObjective Obj;
		Obj.Description = FText::FromString(TEXT("Delivery complete. Mission records logged."));
		Obj.Type = EObjectiveType::GoToLocation;
		Obj.TargetLocation = FVector(-7650.0f, 9350.0f, 20.0f);
		Obj.AcceptanceRadius = 1000.0f;
		Objectives.Add(Obj);
	}
}
