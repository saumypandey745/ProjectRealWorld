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

	/** Toggle touch controls overlay force-display (for editor mobile preview) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Mobile")
	bool bForceShowTouchControls = false;

protected:
	virtual void BeginPlay() override;

	void DrawMinimalVitals(ARealWorldCharacter* Char, ARealWorldPlayerState* PS, float UIScale, float SafeInsetX);
	void DrawVehicleHUD(ARealWorldVehicleBase* Vehicle, float UIScale, float SafeInsetX);
	void DrawWantedIndicator(int32 WantedLevel, float UIScale, float SafeInsetX);
	void DrawInteractionReticle();
	void DrawMissionDisplay(float UIScale, float SafeInsetX);

	// --- Mobile Touch Controls ---
	void DrawMobileTouchControls(bool bIsInVehicle, float UIScale, float SafeInsetX);
	void DrawMobileVirtualJoystick(const FVector2D& Center, float Radius);
	void DrawMobileActionButton(const FVector2D& Position, float Radius, const FString& Label, const FLinearColor& Color, bool bIsSubAction = false);

#if !UE_BUILD_SHIPPING
	void DrawDevelopmentDebugOverlay(ARealWorldCharacter* Char, ARealWorldPlayerState* PS, float UIScale, float SafeInsetX);
#endif

private:
	FText CurrentInteractionPrompt;
	bool bHasInteractionPrompt = false;
};
