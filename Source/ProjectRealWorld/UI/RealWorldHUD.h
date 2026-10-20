#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RealWorldHUD.generated.h"

class ARealWorldCharacter;
class ARealWorldPlayerState;
class ARealWorldVehicleBase;

UCLASS()
class PROJECTREALWORLD_API ARealWorldHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARealWorldHUD();

	virtual void DrawHUD() override;

	/** Set active interaction prompt displayed in center reticle */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetInteractionPrompt(const FText& NewPrompt);

	/** Clear interaction prompt */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ClearInteractionPrompt();

	/** Toggle development debug metrics overlay (non-shipping builds only) */
	UFUNCTION(BlueprintCallable, Exec, Category = "HUD|Debug")
	void ToggleDebugOverlay();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Debug")
	bool bShowDebugOverlay = true;

protected:
	virtual void BeginPlay() override;

	void DrawMinimalVitals(ARealWorldCharacter* Char, ARealWorldPlayerState* PS);
	void DrawVehicleHUD(ARealWorldVehicleBase* Vehicle);
	void DrawWantedIndicator(int32 WantedLevel);
	void DrawInteractionReticle();
	void DrawMissionDisplay();

#if !UE_BUILD_SHIPPING
	void DrawDevelopmentDebugOverlay(ARealWorldCharacter* Char, ARealWorldPlayerState* PS);
#endif

private:
	FText CurrentInteractionPrompt;
	bool bHasInteractionPrompt = false;
};
