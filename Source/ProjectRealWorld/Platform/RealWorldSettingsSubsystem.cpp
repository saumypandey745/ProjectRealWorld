#include "Platform/RealWorldSettingsSubsystem.h"
#include "Platform/RealWorldPlatformSubsystem.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"

void URealWorldSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadSettingsFromDisk();
}

void URealWorldSettingsSubsystem::SetGraphicsPreset(ERealWorldGraphicsPreset Preset)
{
	ActiveSettings.ActivePreset = Preset;

	if (URealWorldPlatformSubsystem* PlatformSub = GetGameInstance()->GetSubsystem<URealWorldPlatformSubsystem>())
	{
		PlatformSub->ApplyGraphicsPreset(Preset);
	}

	ApplySettingsToEngine();
}

void URealWorldSettingsSubsystem::SetResolutionScale(float NewScale)
{
	ActiveSettings.ResolutionScale = FMath::Clamp(NewScale, 50.0f, 150.0f);
	ApplySettingsToEngine();
}

void URealWorldSettingsSubsystem::SetViewDistanceQuality(int32 Quality)
{
	ActiveSettings.ViewDistanceQuality = FMath::Clamp(Quality, 0, 4);
	ApplySettingsToEngine();
}

void URealWorldSettingsSubsystem::SetShadowQuality(int32 Quality)
{
	ActiveSettings.ShadowQuality = FMath::Clamp(Quality, 0, 4);
	ApplySettingsToEngine();
}

void URealWorldSettingsSubsystem::SetPostProcessQuality(int32 Quality)
{
	ActiveSettings.PostProcessQuality = FMath::Clamp(Quality, 0, 4);
	ApplySettingsToEngine();
}

void URealWorldSettingsSubsystem::SetTextureQuality(int32 Quality)
{
	ActiveSettings.TextureQuality = FMath::Clamp(Quality, 0, 4);
	ApplySettingsToEngine();
}

void URealWorldSettingsSubsystem::SetEffectsQuality(int32 Quality)
{
	ActiveSettings.EffectsQuality = FMath::Clamp(Quality, 0, 4);
	ApplySettingsToEngine();
}

void URealWorldSettingsSubsystem::SetFoliageQuality(int32 Quality)
{
	ActiveSettings.FoliageQuality = FMath::Clamp(Quality, 0, 4);
	ApplySettingsToEngine();
}

void URealWorldSettingsSubsystem::SetGlobalIlluminationQuality(int32 Quality)
{
	ActiveSettings.GlobalIlluminationQuality = FMath::Clamp(Quality, 0, 4);
	ApplySettingsToEngine();
}

void URealWorldSettingsSubsystem::SetReflectionQuality(int32 Quality)
{
	ActiveSettings.ReflectionQuality = FMath::Clamp(Quality, 0, 4);
	ApplySettingsToEngine();
}

void URealWorldSettingsSubsystem::SetTargetFPS(int32 NewFPS)
{
	ActiveSettings.TargetFPS = NewFPS;
	ApplySettingsToEngine();
}

void URealWorldSettingsSubsystem::SetNPCDensityMultiplier(float Multiplier)
{
	ActiveSettings.NPCDensityMultiplier = FMath::Clamp(Multiplier, 0.25f, 1.5f);
	OnSettingsUpdated.Broadcast(ActiveSettings);
}

void URealWorldSettingsSubsystem::SetTrafficDensityMultiplier(float Multiplier)
{
	ActiveSettings.TrafficDensityMultiplier = FMath::Clamp(Multiplier, 0.25f, 1.5f);
	OnSettingsUpdated.Broadcast(ActiveSettings);
}

void URealWorldSettingsSubsystem::SetVSyncEnabled(bool bEnabled)
{
	ActiveSettings.bEnableVSync = bEnabled;
	ApplySettingsToEngine();
}

void URealWorldSettingsSubsystem::SetBatteryThermalMode(EBatteryThermalMode Mode)
{
	ActiveSettings.BatteryMode = Mode;

	if (URealWorldPlatformSubsystem* PlatformSub = GetGameInstance()->GetSubsystem<URealWorldPlatformSubsystem>())
	{
		PlatformSub->SetBatteryThermalMode(Mode);
	}

	OnSettingsUpdated.Broadcast(ActiveSettings);
}

void URealWorldSettingsSubsystem::ResetToPlatformDefaults()
{
	if (URealWorldPlatformSubsystem* PlatformSub = GetGameInstance()->GetSubsystem<URealWorldPlatformSubsystem>())
	{
		SetGraphicsPreset(PlatformSub->GetRecommendedGraphicsPreset());
	}
	else
	{
		SetGraphicsPreset(ERealWorldGraphicsPreset::PC_High);
	}
}

void URealWorldSettingsSubsystem::SaveSettingsToDisk()
{
	if (GEngine)
	{
		if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
		{
			UserSettings->SaveSettings();
		}
	}
}

void URealWorldSettingsSubsystem::LoadSettingsFromDisk()
{
	if (GEngine)
	{
		if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
		{
			UserSettings->LoadSettings();

			ActiveSettings.ViewDistanceQuality = UserSettings->GetViewDistanceQuality();
			ActiveSettings.AntiAliasingQuality = UserSettings->GetAntiAliasingQuality();
			ActiveSettings.ShadowQuality = UserSettings->GetShadowQuality();
			ActiveSettings.PostProcessQuality = UserSettings->GetPostProcessingQuality();
			ActiveSettings.TextureQuality = UserSettings->GetTextureQuality();
			ActiveSettings.EffectsQuality = UserSettings->GetVisualEffectQuality();
			ActiveSettings.FoliageQuality = UserSettings->GetFoliageQuality();
			ActiveSettings.GlobalIlluminationQuality = UserSettings->GetGlobalIlluminationQuality();
			ActiveSettings.ReflectionQuality = UserSettings->GetReflectionQuality();
			ActiveSettings.bEnableVSync = UserSettings->IsVSyncEnabled();
			ActiveSettings.TargetFPS = FMath::RoundToInt(UserSettings->GetFrameRateLimit());
		}
	}
}

void URealWorldSettingsSubsystem::ApplySettingsToEngine()
{
	if (!GEngine) return;

	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
	if (UserSettings)
	{
		UserSettings->SetViewDistanceQuality(ActiveSettings.ViewDistanceQuality);
		UserSettings->SetAntiAliasingQuality(ActiveSettings.AntiAliasingQuality);
		UserSettings->SetShadowQuality(ActiveSettings.ShadowQuality);
		UserSettings->SetPostProcessingQuality(ActiveSettings.PostProcessQuality);
		UserSettings->SetTextureQuality(ActiveSettings.TextureQuality);
		UserSettings->SetVisualEffectQuality(ActiveSettings.EffectsQuality);
		UserSettings->SetFoliageQuality(ActiveSettings.FoliageQuality);
		UserSettings->SetGlobalIlluminationQuality(ActiveSettings.GlobalIlluminationQuality);
		UserSettings->SetReflectionQuality(ActiveSettings.ReflectionQuality);
		UserSettings->SetVSyncEnabled(ActiveSettings.bEnableVSync);
		UserSettings->SetFrameRateLimit(static_cast<float>(ActiveSettings.TargetFPS));

		UserSettings->ApplySettings(false);
	}

	OnSettingsUpdated.Broadcast(ActiveSettings);
}
