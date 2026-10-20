#include "UI/RealWorldHUD.h"
#include "Character/RealWorldCharacter.h"
#include "Vehicles/RealWorldVehicleBase.h"
#include "Core/RealWorldPlayerState.h"
#include "Core/RealWorldPlayerController.h"
#include "Platform/RealWorldPlatformSubsystem.h"
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
	bForceShowTouchControls = false;
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

	// Calculate responsive UI scale factor based on screen height (baseline 1080p)
	const float UIScale = FMath::Clamp(Canvas->SizeY / 1080.0f, 0.65f, 1.35f);
	// Safe insets to handle mobile display cutouts/notches (18:9, 19.5:9, 20:9)
	const float SafeInsetX = FMath::Max(24.0f, Canvas->SizeX * 0.035f);

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	ARealWorldCharacter* Char = Cast<ARealWorldCharacter>(Pawn);
	ARealWorldVehicleBase* Vehicle = Cast<ARealWorldVehicleBase>(Pawn);
	ARealWorldPlayerController* PC = Cast<ARealWorldPlayerController>(GetOwningPlayerController());
	ARealWorldPlayerState* PS = PC ? PC->GetPlayerState<ARealWorldPlayerState>() : nullptr;

	bool bIsMobile = bForceShowTouchControls;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (URealWorldPlatformSubsystem* PlatformSub = GI->GetSubsystem<URealWorldPlatformSubsystem>())
		{
			bIsMobile = bIsMobile || PlatformSub->IsMobilePlatform() || (PC && PC->bIsUsingMobileTouchInput);
		}
	}

	if (Vehicle)
	{
		DrawVehicleHUD(Vehicle, UIScale, SafeInsetX);
		if (PS)
		{
			DrawWantedIndicator(PS->WantedLevel, UIScale, SafeInsetX);
		}
		if (bIsMobile)
		{
			DrawMobileTouchControls(true, UIScale, SafeInsetX);
		}
	}
	else if (Char && PS)
	{
		DrawMinimalVitals(Char, PS, UIScale, SafeInsetX);
		DrawWantedIndicator(PS->WantedLevel, UIScale, SafeInsetX);

		if (bIsMobile)
		{
			DrawMobileTouchControls(false, UIScale, SafeInsetX);
		}

#if !UE_BUILD_SHIPPING
		if (bShowDebugOverlay && !bIsMobile)
		{
			DrawDevelopmentDebugOverlay(Char, PS, UIScale, SafeInsetX);
		}
#endif
	}

	DrawInteractionReticle();
	DrawMissionDisplay(UIScale, SafeInsetX);
}

void ARealWorldHUD::DrawMinimalVitals(ARealWorldCharacter* Char, ARealWorldPlayerState* PS, float UIScale, float SafeInsetX)
{
	const float ScreenHeight = Canvas->SizeY;
	const float OriginX = SafeInsetX + (10.0f * UIScale);
	const float OriginY = ScreenHeight - (95.0f * UIScale);
	const float BarWidth = 190.0f * UIScale;
	const float BarHeight = 7.0f * UIScale;

	// Backing tile
	FCanvasTileItem BackingTile(FVector2D(OriginX - (10.0f * UIScale), OriginY - (28.0f * UIScale)), FVector2D((BarWidth + 140.0f) * UIScale, 85.0f * UIScale), FLinearColor(0.02f, 0.02f, 0.04f, 0.65f));
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
	FCanvasTileItem StaminaBg(FVector2D(OriginX, OriginY + (12.0f * UIScale)), FVector2D(BarWidth, BarHeight * 0.7f), FLinearColor(0.1f, 0.1f, 0.2f, 0.8f));
	FCanvasTileItem StaminaFg(FVector2D(OriginX, OriginY + (12.0f * UIScale)), FVector2D(BarWidth * StaminaPercent, BarHeight * 0.7f), StaminaColor);
	Canvas->DrawItem(StaminaBg);
	Canvas->DrawItem(StaminaFg);

	// Cash Counter
	FString CashStr = FString::Printf(TEXT("$%'.0f"), PS->Cash);
	FCanvasTextItem CashText(FVector2D(OriginX + BarWidth + (16.0f * UIScale), OriginY - (4.0f * UIScale)), FText::FromString(CashStr), GEngine->GetMediumFont(), FLinearColor(0.35f, 0.95f, 0.45f, 0.95f));
	CashText.Scale = FVector2D(UIScale, UIScale);
	Canvas->DrawItem(CashText);

	// Clock
	if (UWorld* World = GetWorld())
	{
		if (UTimeOfDaySubsystem* TimeSub = World->GetSubsystem<UTimeOfDaySubsystem>())
		{
			FCanvasTextItem ClockText(FVector2D(OriginX, OriginY - (22.0f * UIScale)), FText::FromString(TimeSub->GetFormattedTimeString()), GEngine->GetSmallFont(), FLinearColor(0.85f, 0.85f, 0.9f, 0.85f));
			ClockText.Scale = FVector2D(UIScale, UIScale);
			Canvas->DrawItem(ClockText);
		}
	}
}

void ARealWorldHUD::DrawVehicleHUD(ARealWorldVehicleBase* Vehicle, float UIScale, float SafeInsetX)
{
	const float ScreenWidth = Canvas->SizeX;
	const float ScreenHeight = Canvas->SizeY;

	const float ClusterW = 250.0f * UIScale;
	const float ClusterH = 100.0f * UIScale;
	const float OriginX = ScreenWidth - ClusterW - SafeInsetX;
	const float OriginY = ScreenHeight - ClusterH - (30.0f * UIScale);

	// Instrument backing
	FCanvasTileItem Backing(FVector2D(OriginX, OriginY), FVector2D(ClusterW, ClusterH), FLinearColor(0.02f, 0.02f, 0.04f, 0.75f));
	Backing.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Backing);

	// Speedometer
	const float SpeedKmH = Vehicle->GetForwardSpeedKmH();
	FString SpeedStr = FString::Printf(TEXT("%.0f"), SpeedKmH);
	FCanvasTextItem SpeedText(FVector2D(OriginX + (14.0f * UIScale), OriginY + (12.0f * UIScale)), FText::FromString(SpeedStr), GEngine->GetLargeFont(), FLinearColor::White);
	SpeedText.Scale = FVector2D(UIScale, UIScale);
	Canvas->DrawItem(SpeedText);

	FCanvasTextItem UnitText(FVector2D(OriginX + (75.0f * UIScale), OriginY + (24.0f * UIScale)), FText::FromString(TEXT("KM/H")), GEngine->GetSmallFont(), FLinearColor(0.7f, 0.7f, 0.75f, 0.9f));
	UnitText.Scale = FVector2D(UIScale, UIScale);
	Canvas->DrawItem(UnitText);

	// Gear
	int32 Gear = Vehicle->GetCurrentGear();
	FString GearStr = (Gear == -1) ? TEXT("R") : ((Gear == 0) ? TEXT("N") : FString::Printf(TEXT("D%d"), Gear));
	FCanvasTextItem GearText(FVector2D(OriginX + (150.0f * UIScale), OriginY + (18.0f * UIScale)), FText::FromString(GearStr), GEngine->GetMediumFont(), FLinearColor(0.3f, 0.85f, 1.0f, 0.95f));
	GearText.Scale = FVector2D(UIScale, UIScale);
	Canvas->DrawItem(GearText);

	// Fuel Bar
	const float GaugeWidth = 130.0f * UIScale;
	const float GaugeHeight = 5.0f * UIScale;
	float FuelPct = Vehicle->GetFuelPercent();
	FCanvasTileItem FuelBg(FVector2D(OriginX + (55.0f * UIScale), OriginY + (60.0f * UIScale)), FVector2D(GaugeWidth, GaugeHeight), FLinearColor(0.15f, 0.15f, 0.15f, 0.8f));
	FCanvasTileItem FuelFg(FVector2D(OriginX + (55.0f * UIScale), OriginY + (60.0f * UIScale)), FVector2D(GaugeWidth * FuelPct, GaugeHeight), FLinearColor(0.95f, 0.75f, 0.1f, 0.95f));
	Canvas->DrawItem(FuelBg);
	Canvas->DrawItem(FuelFg);

	// Health Bar
	float HealthPct = Vehicle->GetHealthPercent();
	FCanvasTileItem HealthBg(FVector2D(OriginX + (55.0f * UIScale), OriginY + (78.0f * UIScale)), FVector2D(GaugeWidth, GaugeHeight), FLinearColor(0.15f, 0.15f, 0.15f, 0.8f));
	FLinearColor ConditionColor = (HealthPct > 0.5f) ? FLinearColor(0.2f, 0.85f, 0.3f, 0.95f) : (HealthPct > 0.25f ? FLinearColor(0.9f, 0.6f, 0.1f, 0.95f) : FLinearColor(0.9f, 0.15f, 0.15f, 0.95f));
	FCanvasTileItem HealthFg(FVector2D(OriginX + (55.0f * UIScale), OriginY + (78.0f * UIScale)), FVector2D(GaugeWidth * HealthPct, GaugeHeight), ConditionColor);
	Canvas->DrawItem(HealthBg);
	Canvas->DrawItem(HealthFg);
}

void ARealWorldHUD::DrawWantedIndicator(int32 WantedLevel, float UIScale, float SafeInsetX)
{
	if (WantedLevel <= 0) return;

	const float ScreenWidth = Canvas->SizeX;
	const float StarSize = 15.0f * UIScale;
	const float Spacing = 5.0f * UIScale;
	const float StartX = ScreenWidth - SafeInsetX - (5 * (StarSize + Spacing));
	const float StartY = 35.0f * UIScale;

	for (int32 i = 0; i < 5; ++i)
	{
		FLinearColor StarColor = (i < WantedLevel) ? FLinearColor(1.0f, 0.82f, 0.05f, 1.0f) : FLinearColor(0.25f, 0.25f, 0.25f, 0.4f);
		FCanvasTileItem StarTile(FVector2D(StartX + (i * (StarSize + Spacing)), StartY), FVector2D(StarSize, StarSize), StarColor);
		StarTile.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(StarTile);
	}

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
				StatusStr = FString::Printf(TEXT("SEARCHING (R=%.0fm)"), WantedSub->CurrentSearchRadius / 100.0f);
				StatusColor = FLinearColor(1.0f, 0.65f, 0.1f, 1.0f);
				break;
			case EPursuitState::Cooldown:
				StatusStr = FString::Printf(TEXT("EVADING [%.0fs]"), WantedSub->EvasionTimer);
				StatusColor = FLinearColor(0.4f, 0.8f, 1.0f, 1.0f);
				break;
			default:
				break;
			}

			if (!StatusStr.IsEmpty())
			{
				FCanvasTextItem StatusText(FVector2D(StartX - (10.0f * UIScale), StartY + (20.0f * UIScale)), FText::FromString(StatusStr), GEngine->GetSmallFont(), StatusColor);
				StatusText.Scale = FVector2D(UIScale, UIScale);
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

void ARealWorldHUD::DrawMissionDisplay(float UIScale, float SafeInsetX)
{
	if (UWorld* World = GetWorld())
	{
		if (UMissionSubsystem* MissionSub = World->GetSubsystem<UMissionSubsystem>())
		{
			if (MissionSub->ActiveMission)
			{
				const float TopX = SafeInsetX + (10.0f * UIScale);
				const float TopY = 35.0f * UIScale;

				FCanvasTextItem MissionTitle(FVector2D(TopX, TopY), MissionSub->ActiveMission->Title, GEngine->GetMediumFont(), FLinearColor(1.0f, 0.8f, 0.2f, 1.0f));
				MissionTitle.Scale = FVector2D(UIScale, UIScale);
				Canvas->DrawItem(MissionTitle);

				if (MissionSub->ActiveMission->Objectives.IsValidIndex(MissionSub->CurrentObjectiveIndex))
				{
					const FMissionObjective& CurObj = MissionSub->ActiveMission->Objectives[MissionSub->CurrentObjectiveIndex];
					FCanvasTextItem ObjectiveText(FVector2D(TopX, TopY + (22.0f * UIScale)), CurObj.Description, GEngine->GetSmallFont(), FLinearColor(0.9f, 0.9f, 0.95f, 0.9f));
					ObjectiveText.Scale = FVector2D(UIScale, UIScale);
					Canvas->DrawItem(ObjectiveText);

					APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
					if (Pawn && CurObj.Type == EObjectiveType::GoToLocation && !CurObj.TargetLocation.IsZero())
					{
						float DistanceMeters = FVector::Dist2D(Pawn->GetActorLocation(), CurObj.TargetLocation) / 100.0f;
						FString DistStr = FString::Printf(TEXT("Waypoint: %.0f m"), DistanceMeters);
						FCanvasTextItem DistText(FVector2D(TopX, TopY + (42.0f * UIScale)), FText::FromString(DistStr), GEngine->GetSmallFont(), FLinearColor(0.35f, 0.85f, 1.0f, 0.95f));
						DistText.Scale = FVector2D(UIScale, UIScale);
						Canvas->DrawItem(DistText);
					}
				}
			}
		}
	}
}

void ARealWorldHUD::DrawMobileTouchControls(bool bIsInVehicle, float UIScale, float SafeInsetX)
{
	const float ScreenWidth = Canvas->SizeX;
	const float ScreenHeight = Canvas->SizeY;

	// 1. Virtual Thumbstick in Lower-Left Quadrant
	const FVector2D StickCenter(SafeInsetX + (110.0f * UIScale), ScreenHeight - (120.0f * UIScale));
	const float StickRadius = 75.0f * UIScale;
	DrawMobileVirtualJoystick(StickCenter, StickRadius);

	// 2. Action Buttons Cluster in Lower-Right Quadrant
	const float BtnRadius = 32.0f * UIScale;
	const float RightOriginX = ScreenWidth - SafeInsetX;
	const float BottomOriginY = ScreenHeight - (40.0f * UIScale);

	if (!bIsInVehicle)
	{
		// On-Foot Touch Buttons
		DrawMobileActionButton(FVector2D(RightOriginX - (60.0f * UIScale), BottomOriginY - (110.0f * UIScale)), BtnRadius, TEXT("JUMP"), FLinearColor(0.2f, 0.7f, 1.0f, 0.85f));
		DrawMobileActionButton(FVector2D(RightOriginX - (130.0f * UIScale), BottomOriginY - (60.0f * UIScale)), BtnRadius, TEXT("SPRINT"), FLinearColor(0.95f, 0.6f, 0.1f, 0.85f));
		DrawMobileActionButton(FVector2D(RightOriginX - (60.0f * UIScale), BottomOriginY - (40.0f * UIScale)), BtnRadius * 0.85f, TEXT("CROUCH"), FLinearColor(0.5f, 0.5f, 0.6f, 0.75f), true);
		DrawMobileActionButton(FVector2D(RightOriginX - (130.0f * UIScale), BottomOriginY - (150.0f * UIScale)), BtnRadius, TEXT("ATTACK"), FLinearColor(0.9f, 0.2f, 0.2f, 0.85f));
		DrawMobileActionButton(FVector2D(RightOriginX - (190.0f * UIScale), BottomOriginY - (95.0f * UIScale)), BtnRadius * 0.9f, TEXT("INTERACT"), FLinearColor(0.3f, 0.9f, 0.4f, 0.85f));
	}
	else
	{
		// In-Vehicle Touch Pedals
		DrawMobileActionButton(FVector2D(RightOriginX - (60.0f * UIScale), BottomOriginY - (90.0f * UIScale)), BtnRadius * 1.15f, TEXT("GAS"), FLinearColor(0.2f, 0.85f, 0.3f, 0.9f));
		DrawMobileActionButton(FVector2D(RightOriginX - (140.0f * UIScale), BottomOriginY - (60.0f * UIScale)), BtnRadius * 1.05f, TEXT("BRAKE"), FLinearColor(0.9f, 0.25f, 0.2f, 0.9f));
		DrawMobileActionButton(FVector2D(RightOriginX - (120.0f * UIScale), BottomOriginY - (150.0f * UIScale)), BtnRadius * 0.85f, TEXT("HANDBRK"), FLinearColor(0.95f, 0.8f, 0.1f, 0.85f));
		DrawMobileActionButton(FVector2D(RightOriginX - (190.0f * UIScale), BottomOriginY - (95.0f * UIScale)), BtnRadius * 0.9f, TEXT("EXIT"), FLinearColor(0.7f, 0.7f, 0.7f, 0.85f));
	}
}

void ARealWorldHUD::DrawMobileVirtualJoystick(const FVector2D& Center, float Radius)
{
	// Outer boundary ring
	FCanvasTileItem OuterRing(Center - FVector2D(Radius, Radius), FVector2D(Radius * 2.0f, Radius * 2.0f), FLinearColor(0.1f, 0.12f, 0.18f, 0.45f));
	OuterRing.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(OuterRing);

	// Inner touch thumbstick nub
	const float NubRadius = Radius * 0.4f;
	FCanvasTileItem InnerNub(Center - FVector2D(NubRadius, NubRadius), FVector2D(NubRadius * 2.0f, NubRadius * 2.0f), FLinearColor(0.85f, 0.9f, 1.0f, 0.65f));
	InnerNub.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(InnerNub);
}

void ARealWorldHUD::DrawMobileActionButton(const FVector2D& Position, float Radius, const FString& Label, const FLinearColor& Color, bool bIsSubAction)
{
	// Button disc
	FCanvasTileItem Disc(Position - FVector2D(Radius, Radius), FVector2D(Radius * 2.0f, Radius * 2.0f), Color);
	Disc.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Disc);

	// Button label centered
	FCanvasTextItem LabelText(Position - FVector2D(Radius * 0.7f, Radius * 0.25f), FText::FromString(Label), GEngine->GetSmallFont(), FLinearColor::White);
	LabelText.Scale = FVector2D(0.85f, 0.85f);
	Canvas->DrawItem(LabelText);
}

#if !UE_BUILD_SHIPPING
void ARealWorldHUD::DrawDevelopmentDebugOverlay(ARealWorldCharacter* Char, ARealWorldPlayerState* PS, float UIScale, float SafeInsetX)
{
	const float CardX = Canvas->SizeX - (300.0f * UIScale) - SafeInsetX;
	const float CardY = 80.0f * UIScale;
	const float CardW = 290.0f * UIScale;
	const float CardH = 210.0f * UIScale;

	FCanvasTileItem CardBg(FVector2D(CardX, CardY), FVector2D(CardW, CardH), FLinearColor(0.01f, 0.01f, 0.02f, 0.85f));
	CardBg.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(CardBg);

	float CurrentLineY = CardY + (8.0f * UIScale);
	auto DrawDebugLine = [this, CardX, &CurrentLineY, UIScale](const FString& Label, const FString& Value, const FLinearColor& ValueColor)
	{
		FCanvasTextItem LabelItem(FVector2D(CardX + (8.0f * UIScale), CurrentLineY), FText::FromString(Label), GEngine->GetSmallFont(), FLinearColor(0.7f, 0.7f, 0.75f, 0.9f));
		FCanvasTextItem ValueItem(FVector2D(CardX + (120.0f * UIScale), CurrentLineY), FText::FromString(Value), GEngine->GetSmallFont(), ValueColor);
		LabelItem.Scale = FVector2D(UIScale, UIScale);
		ValueItem.Scale = FVector2D(UIScale, UIScale);
		Canvas->DrawItem(LabelItem);
		Canvas->DrawItem(ValueItem);
		CurrentLineY += 16.0f * UIScale;
	};

	FCanvasTextItem HeaderText(FVector2D(CardX + (8.0f * UIScale), CurrentLineY), FText::FromString(TEXT("=== REALWORLD DEBUG METRICS ===")), GEngine->GetSmallFont(), FLinearColor(1.0f, 0.75f, 0.2f, 1.0f));
	HeaderText.Scale = FVector2D(UIScale, UIScale);
	Canvas->DrawItem(HeaderText);
	CurrentLineY += 18.0f * UIScale;

	const UEnum* LocEnum = StaticEnum<ELocomotionState>();
	FString StateStr = LocEnum ? LocEnum->GetNameStringByValue((int64)Char->CurrentLocomotionState) : TEXT("Unknown");
	DrawDebugLine(TEXT("Movement:"), StateStr, FLinearColor(0.3f, 0.9f, 0.4f, 1.0f));

	const float SpeedKmH = Char->GetVelocity().Size2D() * 0.036f;
	DrawDebugLine(TEXT("Speed:"), FString::Printf(TEXT("%.1f km/h"), SpeedKmH), FLinearColor::White);

	DrawDebugLine(TEXT("Stamina:"), FString::Printf(TEXT("%.0f / %.0f"), Char->CurrentStamina, Char->MaxStamina), Char->bIsExhausted ? FLinearColor::Red : FLinearColor(0.3f, 0.8f, 1.0f, 1.0f));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (URealWorldPlatformSubsystem* PlatformSub = GI->GetSubsystem<URealWorldPlatformSubsystem>())
		{
			const UEnum* TierEnum = StaticEnum<EPlatformPerformanceTier>();
			FString TierStr = TierEnum ? TierEnum->GetNameStringByValue((int64)PlatformSub->GetPerformanceTier()) : TEXT("Unknown");
			DrawDebugLine(TEXT("Tier:"), TierStr, FLinearColor(0.9f, 0.85f, 0.2f, 1.0f));
		}
	}
}
#endif
