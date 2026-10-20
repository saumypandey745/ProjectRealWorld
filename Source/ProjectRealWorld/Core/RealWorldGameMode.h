#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RealWorldGameMode.generated.h"

UCLASS()
class PROJECTREALWORLD_API ARealWorldGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARealWorldGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;

	/** Starting cash balance granted to player on fresh game */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode|Economy")
	float InitialPlayerCash = 1500.0f;

	/** Starting wanted level on fresh session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode|Law")
	int32 StartingWantedLevel = 0;
};
