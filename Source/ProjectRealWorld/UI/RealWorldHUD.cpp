#include "UI/RealWorldHUD.h"
#include "Character/RealWorldCharacter.h"
#include "Vehicles/RealWorldVehicleBase.h"
#include "Core/RealWorldPlayerState.h"
#include "Interaction/RealWorldInteractionComponent.h"
#include "Subsystems/TimeOfDaySubsystem.h"
#include "Subsystems/WeatherSubsystem.h"
#include "Subsystems/MissionSubsystem.h"
#include "Subsystems/PoliceWantedSubsystem.h"
#include "World/RealWorldSpawnerSubsystem.h"
#include "Engine/Canvas.h"
#include "Kismet/GameplayStatics.h"

ARealWorldHUD::ARealWorldHUD()
{
	bHasInteractionPrompt = false;
	bShowDebugOverlay = true;
}

void ARealWorldHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ARealWorldHUD::SetInteractionPrompt(const FText& NewPrompt)
{
	CurrentInteractionPrompt = NewPrompt;
	bHasInteractionPrompt = !NewPrompt.IsEmpty();
}

void ARealWorldHUD::ClearInteractionPrompt()
{
	CurrentInteractionPrompt = FText::GetEmpty();
	bHasInteractionPrompt = false;
}

void ARealWorldHUD::ToggleDebugOverlay()
{
	bShowDebugOverlay = !bShowDebugOverlay;
}

void ARealWorldHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas) return;

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	ARealWorldCharacter* Char = Cast<ARealWorldCharacter>(Pawn);
	ARealWorldVehicleBase* Vehicle = Cast<ARealWorldVehicleBase>(Pawn);
	ARealWorldPlayerState* PS = GetOwningPlayerController() ? GetOwningPlayerController()->GetPlayerState<ARealWorldPlayerState>() : nullptr;

	if (Vehicle)
	{
		DrawVehicleHUD(Vehicle);
		if (PS)
		{
			DrawWantedIndicator(PS->WantedLevel);
		}
	}
	else if (Char && PS)
	{
		DrawMinimalVitals(Char, PS);
		DrawWantedIndicator(PS->WantedLevel);

#if !UE_BUILD_SHIPPING
		if (bShowDebugOverlay)
		{
			DrawDevelopmentDebugOverlay(Char, PS);
		}
#endif
	}

	DrawInteractionReticle();
	DrawMissionDisplay();
}

void ARealWorldHUD::DrawMinimalVitals(ARealWorldCharacter* Char, ARealWorldPlayerState* PS)
{
	const float ScreenWidth = Canvas->SizeX;
	const float ScreenHeight = Canvas->SizeY;

	const float OriginX = 45.0f;
	const float OriginY = ScreenHeight - 110.0f;
	const float BarWidth = 190.0f;
	const float BarHeight = 7.0f;

	// Backing
	FCanvasTileItem BackingTile(FVector2D(OriginX - 10.0f, OriginY - 28.0f), FVector2D(BarWidth + 140.0f, 90.0f), FLinearColor(0.02f, 0.02f, 0.04f, 0.65f));
	BackingTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BackingTile);

	// Health Bar
	float HealthPercent = FMath::Clamp(Char->CurrentHealth / FMath::Max(1.0f, Char->MaxHealth), 0.0f, 1.0f);
	FCanvasTileItem HealthBg(FVector2D(OriginX, OriginY), FVector2D(BarWidth, BarHeight), FLinearColor(0.1f, 0.15f, 0.1f, 0.8f));
	FCanvasTileItem HealthFg(FVector2D(OriginX, OriginY), FVector2D(BarWidth * HealthPercent, BarHeight), FLinearColor(0.18f, 0.82f, 0.35f, 0.95f));
	Canvas->DrawItem(HealthBg);
	Canvas->DrawItem(HealthFg);

	// Stamina Bar
	float StaminaPercent = FMath::Clamp(Char->CurrentStamina / FMath::Max(1.0f, Char->MaxStamina), 0.0f, 1.0f);
	FLinearColor StaminaColor = Char->bIsExhausted ? FLinearColor(0.95f, 0.25f, 0.2f, 0.9f) : FLinearColor(0.2f, 0.65f, 0.95f, 0.9f);
	FCanvasTileItem StaminaBg(FVector2D(OriginX, OriginY + 12.0f), FVector2D(BarWidth, BarHeight * 0.7f), FLinearColor(0.1f, 0.1f, 0.2f, 0.8f));
	FCanvasTileItem StaminaFg(FVector2D(OriginX, OriginY + 12.0f), FVector2D(BarWidth * StaminaPercent, BarHeight * 0.7f), StaminaColor);
	Canvas->DrawItem(StaminaBg);
	Canvas->DrawItem(StaminaFg);

	// Cash Counter
	FString CashStr = FString::Printf(TEXT("$%'.0f"), PS->Cash);
	FCanvasTextItem CashText(FVector2D(OriginX + BarWidth + 20.0f, OriginY - 4.0f), FText::FromString(CashStr), GEngine->GetMediumFont(), FLinearColor(0.35f, 0.95f, 0.45f, 0.95f));
	Canvas->DrawItem(CashText);

	// Clock
	if (UWorld* World = GetWorld())
	{
		if (UTimeOfDaySubsystem* TimeSub = World->GetSubsystem<UTimeOfDaySubsystem>())
		{
			FCanvasTextItem ClockText(FVector2D(OriginX, OriginY - 24.0f), FText::FromString(TimeSub->GetFormattedTimeString()), GEngine->GetSmallFont(), FLinearColor(0.85f, 0.85f, 0.9f, 0.85f));
			Canvas->DrawItem(ClockText);
		}
	}
}

void ARealWorldHUD::DrawVehicleHUD(ARealWorldVehicleBase* Vehicle)
{
	const float ScreenWidth = Canvas->SizeX;
	const float ScreenHeight = Canvas->SizeY;

	const float ClusterW = 260.0f;
	const float ClusterH = 110.0f;
	const float OriginX = ScreenWidth - ClusterW - 45.0f;
	const float OriginY = ScreenHeight - ClusterH - 35.0f;

	// Dark semi-transparent instrument backing
	FCanvasTileItem Backing(FVector2D(OriginX, OriginY), FVector2D(ClusterW, ClusterH), FLinearColor(0.02f, 0.02f, 0.04f, 0.75f));
	Backing.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Backing);

	// Speedometer
	const float SpeedKmH = Vehicle->GetForwardSpeedKmH();
	FString SpeedStr = FString::Printf(TEXT("%.0f"), SpeedKmH);
	FCanvasTextItem SpeedText(FVector2D(OriginX + 16.0f, OriginY + 14.0f), FText::FromString(SpeedStr), GEngine->GetLargeFont(), FLinearColor::White);
	Canvas->DrawItem(SpeedText);

	FCanvasTextItem UnitText(FVector2D(OriginX + 85.0f, OriginY + 28.0f), FText::FromString(TEXT("KM/H")), GEngine->GetSmallFont(), FLinearColor(0.7f, 0.7f, 0.75f, 0.9f));
	Canvas->DrawItem(UnitText);

	// Gear
	int32 Gear = Vehicle->GetCurrentGear();
	FString GearStr = (Gear == -1) ? TEXT("R") : ((Gear == 0) ? TEXT("N") : FString::Printf(TEXT("D%d"), Gear));
	FCanvasTextItem GearText(FVector2D(OriginX + 160.0f, OriginY + 20.0f), FText::FromString(GearStr), GEngine->GetMediumFont(), FLinearColor(0.3f, 0.85f, 1.0f, 0.95f));
	Canvas->DrawItem(GearText);

	// Fuel Bar
	const float GaugeWidth = 140.0f;
	const float GaugeHeight = 5.0f;
	float FuelPct = Vehicle->GetFuelPercent();
	FCanvasTextItem FuelLabel(FVector2D(OriginX + 16.0f, OriginY + 62.0f), FText::FromString(TEXT("FUEL")), GEngine->GetSmallFont(), FLinearColor(0.8f, 0.8f, 0.8f, 0.8f));
	Canvas->DrawItem(FuelLabel);

	FCanvasTileItem FuelBg(FVector2D(OriginX + 60.0f, OriginY + 66.0f), FVector2D(GaugeWidth, GaugeHeight), FLinearColor(0.15f, 0.15f, 0.15f, 0.8f));
	FCanvasTileItem FuelFg(FVector2D(OriginX + 60.0f, OriginY + 66.0f), FVector2D(GaugeWidth * FuelPct, GaugeHeight), FLinearColor(0.95f, 0.75f, 0.1f, 0.95f));
	Canvas->DrawItem(FuelBg);
	Canvas->DrawItem(FuelFg);

	// Vehicle Health / Integrity Bar
	float HealthPct = Vehicle->GetHealthPercent();
	FCanvasTextItem HealthLabel(FVector2D(OriginX + 16.0f, OriginY + 82.0f), FText::FromString(TEXT("BODY")), GEngine->GetSmallFont(), FLinearColor(0.8f, 0.8f, 0.8f, 0.8f));
	Canvas->DrawItem(HealthLabel);

	FCanvasTileItem HealthBg(FVector2D(OriginX + 60.0f, OriginY + 86.0f), FVector2D(GaugeWidth, GaugeHeight), FLinearColor(0.15f, 0.15f, 0.15f, 0.8f));
	FLinearColor ConditionColor = (HealthPct > 0.5f) ? FLinearColor(0.2f, 0.85f, 0.3f, 0.95f) : (HealthPct > 0.25f ? FLinearColor(0.9f, 0.6f, 0.1f, 0.95f) : FLinearColor(0.9f, 0.15f, 0.15f, 0.95f));
	FCanvasTileItem HealthFg(FVector2D(OriginX + 60.0f, OriginY + 86.0f), FVector2D(GaugeWidth * HealthPct, GaugeHeight), ConditionColor);
	Canvas->DrawItem(HealthBg);
	Canvas->DrawItem(HealthFg);
}

void ARealWorldHUD::DrawWantedIndicator(int32 WantedLevel)
{
	if (WantedLevel <= 0) return;

	const float ScreenWidth = Canvas->SizeX;
	const float StarSize = 16.0f;
	const float Spacing = 6.0f;
	const float StartX = ScreenWidth - 190.0f;
	const float StartY = 40.0f;

	for (int32 i = 0; i < 5; ++i)
	{
		FLinearColor StarColor = (i < WantedLevel) ? FLinearColor(1.0f, 0.82f, 0.05f, 1.0f) : FLinearColor(0.25f, 0.25f, 0.25f, 0.4f);
		FCanvasTileItem StarTile(FVector2D(StartX + (i * (StarSize + Spacing)), StartY), FVector2D(StarSize, StarSize), StarColor);
		StarTile.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(StarTile);
	}

	// Pursuit / Search Status Label
	if (UWorld* World = GetWorld())
	{
		if (UPoliceWantedSubsystem* WantedSub = World->GetSubsystem<UPoliceWantedSubsystem>())
		{
			FString StatusStr;
			FLinearColor StatusColor = FLinearColor::Yellow;

			switch (WantedSub->CurrentPursuitState)
			{
			case EPursuitState::ActivePursuit:
				StatusStr = TEXT("ACTIVE PURSUIT");
				StatusColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
				break;
			case EPursuitState::SearchingLastKnownArea:
				StatusStr = FString::Printf(TEXT("SEARCHING AREA (R=%.0fm)"), WantedSub->CurrentSearchRadius / 100.0f);
				StatusColor = FLinearColor(1.0f, 0.65f, 0.1f, 1.0f);
				break;
			case EPursuitState::Cooldown:
				StatusStr = FString::Printf(TEXT("EVADING PATROLS [%.0fs]"), WantedSub->EvasionTimer);
				StatusColor = FLinearColor(0.4f, 0.8f, 1.0f, 1.0f);
				break;
			case EPursuitState::Investigating:
				StatusStr = TEXT("POLICE INVESTIGATING");
				StatusColor = FLinearColor(0.9f, 0.85f, 0.2f, 1.0f);
				break;
			default:
				break;
			}

			if (!StatusStr.IsEmpty())
			{
				FCanvasTextItem StatusText(FVector2D(StartX - 20.0f, StartY + 22.0f), FText::FromString(StatusStr), GEngine->GetSmallFont(), StatusColor);
				Canvas->DrawItem(StatusText);
			}
		}
	}
}

void ARealWorldHUD::DrawInteractionReticle()
{
	const float CenterX = Canvas->SizeX * 0.5f;
	const float CenterY = Canvas->SizeY * 0.5f;

	FCanvasTileItem DotTile(FVector2D(CenterX - 2.0f, CenterY - 2.0f), FVector2D(4.0f, 4.0f), FLinearColor(1.0f, 1.0f, 1.0f, 0.7f));
	Canvas->DrawItem(DotTile);

	if (bHasInteractionPrompt)
	{
		FCanvasTextItem PromptText(FVector2D(CenterX - 70.0f, CenterY + 18.0f), CurrentInteractionPrompt, GEngine->GetSmallFont(), FLinearColor(1.0f, 0.92f, 0.35f, 1.0f));
		Canvas->DrawItem(PromptText);
	}
}

void ARealWorldHUD::DrawMissionDisplay()
{
	if (UWorld* World = GetWorld())
	{
		if (UMissionSubsystem* MissionSub = World->GetSubsystem<UMissionSubsystem>())
		{
			if (MissionSub->ActiveMission)
			{
				const float TopX = 45.0f;
				const float TopY = 40.0f;

				FCanvasTextItem MissionTitle(FVector2D(TopX, TopY), MissionSub->ActiveMission->Title, GEngine->GetMediumFont(), FLinearColor(1.0f, 0.8f, 0.2f, 1.0f));
				Canvas->DrawItem(MissionTitle);

				if (MissionSub->ActiveMission->Objectives.IsValidIndex(MissionSub->CurrentObjectiveIndex))
				{
					const FMissionObjective& CurObj = MissionSub->ActiveMission->Objectives[MissionSub->CurrentObjectiveIndex];
					FCanvasTextItem ObjectiveText(FVector2D(TopX, TopY + 24.0f), CurObj.Description, GEngine->GetSmallFont(), FLinearColor(0.9f, 0.9f, 0.95f, 0.9f));
					Canvas->DrawItem(ObjectiveText);

					// Objective Waypoint Distance
					APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
					if (Pawn && CurObj.Type == EObjectiveType::GoToLocation && !CurObj.TargetLocation.IsZero())
					{
						float DistanceMeters = FVector::Dist2D(Pawn->GetActorLocation(), CurObj.TargetLocation) / 100.0f;
						FString DistStr = FString::Printf(TEXT("Waypoint: %.0f m"), DistanceMeters);
						FCanvasTextItem DistText(FVector2D(TopX, TopY + 44.0f), FText::FromString(DistStr), GEngine->GetSmallFont(), FLinearColor(0.35f, 0.85f, 1.0f, 0.95f));
						Canvas->DrawItem(DistText);
					}
				}
			}
		}
	}
}

#if !UE_BUILD_SHIPPING
void ARealWorldHUD::DrawDevelopmentDebugOverlay(ARealWorldCharacter* Char, ARealWorldPlayerState* PS)
{
	const float CardX = Canvas->SizeX - 320.0f;
	const float CardY = 90.0f;
	const float CardW = 300.0f;
	const float CardH = 220.0f;

	// Dark semi-transparent card background
	FCanvasTileItem CardBg(FVector2D(CardX, CardY), FVector2D(CardW, CardH), FLinearColor(0.01f, 0.01f, 0.02f, 0.85f));
	CardBg.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(CardBg);

	float CurrentLineY = CardY + 8.0f;
	auto DrawDebugLine = [this, CardX, &CurrentLineY](const FString& Label, const FString& Value, const FLinearColor& ValueColor)
	{
		FCanvasTextItem LabelItem(FVector2D(CardX + 10.0f, CurrentLineY), FText::FromString(Label), GEngine->GetSmallFont(), FLinearColor(0.7f, 0.7f, 0.75f, 0.9f));
		FCanvasTextItem ValueItem(FVector2D(CardX + 130.0f, CurrentLineY), FText::FromString(Value), GEngine->GetSmallFont(), ValueColor);
		Canvas->DrawItem(LabelItem);
		Canvas->DrawItem(ValueItem);
		CurrentLineY += 18.0f;
	};

	// Header
	FCanvasTextItem HeaderText(FVector2D(CardX + 10.0f, CurrentLineY), FText::FromString(TEXT("=== REALWORLD DEBUG METRICS ===")), GEngine->GetSmallFont(), FLinearColor(1.0f, 0.75f, 0.2f, 1.0f));
	Canvas->DrawItem(HeaderText);
	CurrentLineY += 22.0f;

	// Locomotion state string
	const UEnum* LocEnum = StaticEnum<ELocomotionState>();
	FString StateStr = LocEnum ? LocEnum->GetNameStringByValue((int64)Char->CurrentLocomotionState) : TEXT("Unknown");
	DrawDebugLine(TEXT("Movement State:"), StateStr, FLinearColor(0.3f, 0.9f, 0.4f, 1.0f));

	// Speed
	const float SpeedCmS = Char->GetVelocity().Size2D();
	const float SpeedKmH = SpeedCmS * 0.036f;
	DrawDebugLine(TEXT("Speed:"), FString::Printf(TEXT("%.1f km/h (%.0f cm/s)"), SpeedKmH, SpeedCmS), FLinearColor::White);

	// Stamina
	FString StaminaStr = FString::Printf(TEXT("%.1f / %.0f %s"), Char->CurrentStamina, Char->MaxStamina, Char->bIsExhausted ? TEXT("[EXHAUSTED]") : TEXT(""));
	DrawDebugLine(TEXT("Stamina:"), StaminaStr, Char->bIsExhausted ? FLinearColor::Red : FLinearColor(0.3f, 0.8f, 1.0f, 1.0f));

	// Weather & Road Wetness
	if (UWorld* World = GetWorld())
	{
		if (UWeatherSubsystem* WeatherSub = World->GetSubsystem<UWeatherSubsystem>())
		{
			const UEnum* WeathEnum = StaticEnum<EWeatherType>();
			FString WeathStr = WeathEnum ? WeathEnum->GetNameStringByValue((int64)WeatherSub->CurrentWeather) : TEXT("Unknown");
			DrawDebugLine(TEXT("Weather / Wet:"), FString::Printf(TEXT("%s (%.2f)"), *WeathStr, WeatherSub->RoadWetness), FLinearColor(0.4f, 0.85f, 1.0f, 1.0f));
		}

		if (UTimeOfDaySubsystem* TimeSub = World->GetSubsystem<UTimeOfDaySubsystem>())
		{
			DrawDebugLine(TEXT("Time of Day:"), FString::Printf(TEXT("%s (%s)"), *TimeSub->GetFormattedTimeString(), TimeSub->bIsNightTime ? TEXT("Night") : TEXT("Day")), FLinearColor(1.0f, 0.95f, 0.5f, 1.0f));
		}

		if (URealWorldSpawnerSubsystem* SpawnerSub = World->GetSubsystem<URealWorldSpawnerSubsystem>())
		{
			const UEnum* DistEnum = StaticEnum<EWorldDistrictType>();
			FString DistStr = DistEnum ? DistEnum->GetNameStringByValue((int64)SpawnerSub->CurrentDistrict) : TEXT("Downtown");
			DrawDebugLine(TEXT("Active District:"), DistStr, FLinearColor(0.85f, 0.65f, 1.0f, 1.0f));
		}
	}

	// Focus Actor
	if (Char->InteractionComponent)
	{
		AActor* Focus = Char->InteractionComponent->GetCurrentFocusedActor();
		FString FocusStr = Focus ? Focus->GetName() : TEXT("None");
		DrawDebugLine(TEXT("Interact Target:"), FocusStr, Focus ? FLinearColor::Yellow : FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
	}
}
#endif
