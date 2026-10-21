#include "Core/RealWorldGameMode.h"
#include "Core/RealWorldPlayerState.h"
#include "Core/RealWorldPlayerController.h"
#include "Character/RealWorldCharacter.h"
#include "UI/RealWorldHUD.h"
#include "ProjectRealWorld.h"

ARealWorldGameMode::ARealWorldGameMode()
{
	DefaultPawnClass = ARealWorldCharacter::StaticClass();
	PlayerControllerClass = ARealWorldPlayerController::StaticClass();
	PlayerStateClass = ARealWorldPlayerState::StaticClass();
	HUDClass = ARealWorldHUD::StaticClass();
}

void ARealWorldGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	UE_LOG(LogProjectRealWorld, Log, TEXT("RealWorldGameMode: Initializing World Map %s"), *MapName);
}

#include "World/RealWorldCityBlockBuilder.h"
#include "Kismet/GameplayStatics.h"

void ARealWorldGameMode::StartPlay()
{
	Super::StartPlay();
	UE_LOG(LogProjectRealWorld, Log, TEXT("RealWorldGameMode: StartPlay executed. Systems active."));

	if (UWorld* World = GetWorld())
	{
		if (!UGameplayStatics::GetActorOfClass(World, ARealWorldCityBlockBuilder::StaticClass()))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			World->SpawnActor<ARealWorldCityBlockBuilder>(ARealWorldCityBlockBuilder::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			UE_LOG(LogProjectRealWorld, Log, TEXT("RealWorldGameMode: Master CityBlockBuilder spawned successfully."));
		}
	}
}
