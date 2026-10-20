#include "Subsystems/MissionSubsystem.h"
#include "Core/RealWorldPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

void UMissionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ActiveMission = nullptr;
	CurrentObjectiveIndex = 0;
}

bool UMissionSubsystem::StartMission(UMissionDataAsset* MissionAsset)
{
	if (!MissionAsset || MissionAsset->Objectives.Num() == 0)
	{
		return false;
	}

	// Abort or complete prior mission if active
	if (ActiveMission)
	{
		FailMission(FText::FromString(TEXT("Superseded by new mission")));
	}

	ActiveMission = MissionAsset;
	CurrentObjectiveIndex = 0;

	// Reset objective states
	for (FMissionObjective& Obj : ActiveMission->Objectives)
	{
		Obj.bIsCompleted = false;
	}

	OnMissionStarted.Broadcast(ActiveMission);
	OnMissionObjectiveUpdated.Broadcast(ActiveMission, CurrentObjectiveIndex);
	return true;
}

void UMissionSubsystem::AdvanceObjective()
{
	if (!ActiveMission) return;

	if (ActiveMission->Objectives.IsValidIndex(CurrentObjectiveIndex))
	{
		ActiveMission->Objectives[CurrentObjectiveIndex].bIsCompleted = true;
	}

	CurrentObjectiveIndex++;

	if (CurrentObjectiveIndex >= ActiveMission->Objectives.Num())
	{
		CompleteMission();
	}
	else
	{
		OnMissionObjectiveUpdated.Broadcast(ActiveMission, CurrentObjectiveIndex);
	}
}

void UMissionSubsystem::FailMission(const FText& Reason)
{
	if (!ActiveMission) return;

	UMissionDataAsset* FailedMission = ActiveMission;
	ActiveMission = nullptr;
	CurrentObjectiveIndex = 0;

	OnMissionCompleted.Broadcast(FailedMission, false);
}

void UMissionSubsystem::CompleteMission()
{
	if (!ActiveMission) return;

	UMissionDataAsset* FinishedMission = ActiveMission;
	CompletedMissionIDs.AddUnique(FinishedMission->MissionID);

	// Payout reward to player state
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ARealWorldPlayerState* PS = PC->GetPlayerState<ARealWorldPlayerState>())
		{
			PS->AddCash(FinishedMission->CashReward);
		}
	}

	ActiveMission = nullptr;
	CurrentObjectiveIndex = 0;

	OnMissionCompleted.Broadcast(FinishedMission, true);
}
