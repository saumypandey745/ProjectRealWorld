#include "Platform/RealWorldPlatformSubsystem.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"
#include "RHI.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"

void URealWorldPlatformSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DetectAndroidHardware();

	// Automatically apply initial recommended mobile graphics preset
	ApplyGraphicsPreset(HardwareInfo.RecommendedPreset);
}

void URealWorldPlatformSubsystem::DetectAndroidHardware()
{
	if (bHasInitializedHardware) return;

	// Target Platform: Android Mobile (EditorEnvironment for development preview)
#if WITH_EDITOR
	HardwareInfo.PlatformType = ERealWorldPlatformType::EditorEnvironment;
#else
	HardwareInfo.PlatformType = ERealWorldPlatformType::Android;
#endif
	HardwareInfo.bSupportsTouch = true;

	// Query Android CPU Cores and RAM
	HardwareInfo.PhysicalCores = FPlatformMisc::NumberOfCores();

	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	HardwareInfo.TotalSystemMemoryMB = static_cast<int32>(MemStats.TotalPhysical / (1024 * 1024));

	HardwareInfo.GPUDescription = GRHIAdapterName.IsEmpty() ? TEXT("Android Mobile GPU (Vulkan/GLES)") : GRHIAdapterName;

	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize = FVector2D::ZeroVector;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		HardwareInfo.ScreenResolution = FIntPoint(FMath::RoundToInt(ViewportSize.X), FMath::RoundToInt(ViewportSize.Y));
		if (HardwareInfo.ScreenResolution.Y > 0)
		{
			HardwareInfo.AspectRatio = static_cast<float>(HardwareInfo.ScreenResolution.X) / static_cast<float>(HardwareInfo.ScreenResolution.Y);
		}
	}
	else
	{
		HardwareInfo.ScreenResolution = FIntPoint(1080, 2400); // Standard Android FHD+ screen
		HardwareInfo.AspectRatio = 20.0f / 9.0f;
	}

	// Determine Mobile Performance Tier & Baseline Preset
	HardwareInfo.PerformanceTier = DeterminePerformanceTier();
	HardwareInfo.RecommendedPreset = DetermineRecommendedPreset(HardwareInfo.PerformanceTier);
	ActiveGraphicsPreset = HardwareInfo.RecommendedPreset;

	bHasInitializedHardware = true;
}

EPlatformPerformanceTier URealWorldPlatformSubsystem::DeterminePerformanceTier() const
{
	// Mobile Tiering — Optimized specifically for 4GB, 6GB, and 8GB+ Android devices
	// 4GB RAM phones (actual usable ~3200-3800MB): MobileLow -> 30fps stable
	// 6GB RAM phones (Snapdragon 7xx / Dimensity 700+): MobileMedium -> 45fps
	// 8GB+ flagship phones: MobileHigh -> 60fps
	if (HardwareInfo.TotalSystemMemoryMB >= 7000 && HardwareInfo.PhysicalCores >= 8)
	{
		return EPlatformPerformanceTier::MobileHigh;
	}
	else if (HardwareInfo.TotalSystemMemoryMB >= 5000 && HardwareInfo.PhysicalCores >= 6)
	{
		return EPlatformPerformanceTier::MobileMedium;
	}
	else
	{
		// Default tier for 4GB RAM devices (Snapdragon 680/695, Helio G88/G99, etc.)
		return EPlatformPerformanceTier::MobileLow;
	}
}

ERealWorldGraphicsPreset URealWorldPlatformSubsystem::DetermineRecommendedPreset(EPlatformPerformanceTier Tier) const
{
	switch (Tier)
	{
	case EPlatformPerformanceTier::MobileHigh:
		return ERealWorldGraphicsPreset::Mobile_High;
	case EPlatformPerformanceTier::MobileMedium:
		return ERealWorldGraphicsPreset::Mobile_Medium;
	case EPlatformPerformanceTier::MobileLow:
	default:
		return ERealWorldGraphicsPreset::Mobile_Low;
	}
}

void URealWorldPlatformSubsystem::ApplyGraphicsPreset(ERealWorldGraphicsPreset NewPreset)
{
	if (NewPreset == ERealWorldGraphicsPreset::Auto)
	{
		NewPreset = HardwareInfo.RecommendedPreset;
	}

	ActiveGraphicsPreset = NewPreset;
	ApplyScalabilityCvars(ActiveGraphicsPreset, ActiveBatteryMode);

	OnGraphicsPresetChanged.Broadcast(ActiveGraphicsPreset);
}

void URealWorldPlatformSubsystem::SetBatteryThermalMode(EBatteryThermalMode NewMode)
{
	ActiveBatteryMode = NewMode;
	ApplyScalabilityCvars(ActiveGraphicsPreset, ActiveBatteryMode);

	OnBatteryThermalModeChanged.Broadcast(ActiveBatteryMode);
}

int32 URealWorldPlatformSubsystem::GetScaledCrowdBudget() const
{
	int32 BaseCount = 6;

	switch (ActiveGraphicsPreset)
	{
	case ERealWorldGraphicsPreset::Mobile_High:
		BaseCount = 14;
		break;
	case ERealWorldGraphicsPreset::Mobile_Medium:
		BaseCount = 8;
		break;
	case ERealWorldGraphicsPreset::Mobile_Low:
	default:
		BaseCount = 4; // 4GB phone safe crowd
		break;
	}

	if (ActiveBatteryMode == EBatteryThermalMode::BatterySaver)
	{
		BaseCount = FMath::Max(3, FMath::RoundToInt(BaseCount * 0.6f));
	}

	return BaseCount;
}

int32 URealWorldPlatformSubsystem::GetScaledTrafficDensity() const
{
	int32 BaseVehicles = 4;

	switch (ActiveGraphicsPreset)
	{
	case ERealWorldGraphicsPreset::Mobile_High:
		BaseVehicles = 8;
		break;
	case ERealWorldGraphicsPreset::Mobile_Medium:
		BaseVehicles = 5;
		break;
	case ERealWorldGraphicsPreset::Mobile_Low:
	default:
		BaseVehicles = 2; // 4GB phone safe traffic
		break;
	}

	if (ActiveBatteryMode == EBatteryThermalMode::BatterySaver)
	{
		BaseVehicles = FMath::Max(2, FMath::RoundToInt(BaseVehicles * 0.5f));
	}

	return BaseVehicles;
}

float URealWorldPlatformSubsystem::GetScaledDrawDistanceMultiplier() const
{
	switch (ActiveGraphicsPreset)
	{
	case ERealWorldGraphicsPreset::Mobile_High:
		return 0.75f;
	case ERealWorldGraphicsPreset::Mobile_Medium:
		return 0.55f;
	case ERealWorldGraphicsPreset::Mobile_Low:
	default:
		return 0.40f;
	}
}

int32 URealWorldPlatformSubsystem::GetTargetFrameRate() const
{
	if (ActiveBatteryMode == EBatteryThermalMode::BatterySaver)
	{
		return 30;
	}

	switch (ActiveGraphicsPreset)
	{
	case ERealWorldGraphicsPreset::Mobile_High:
		return 60;
	case ERealWorldGraphicsPreset::Mobile_Medium:
		return 45;
	case ERealWorldGraphicsPreset::Mobile_Low:
	default:
		return 30;
	}
}

void URealWorldPlatformSubsystem::ApplyScalabilityCvars(ERealWorldGraphicsPreset Preset, EBatteryThermalMode ThermalMode)
{
	if (!GEngine) return;

	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();

	switch (Preset)
	{
	case ERealWorldGraphicsPreset::Mobile_High:
		// Mobile Flagship: CSM shadows 2 cascades, 1080p rendering, foliage 0.5
		GEngine->Exec(GetWorld(), TEXT("r.Nanite 0"));
		GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Shadow.Virtual.Enable 0"));
		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 2"));
		GEngine->Exec(GetWorld(), TEXT("r.Shadow.CSM.MaxCascades 2"));
		GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.75"));
		GEngine->Exec(GetWorld(), TEXT("r.Niagara.QualityLevel 1"));
		GEngine->Exec(GetWorld(), TEXT("foliage.DensityScale 0.5"));
		if (UserSettings)
		{
			UserSettings->SetOverallScalabilityLevel(1);
		}
		break;

	case ERealWorldGraphicsPreset::Mobile_Medium:
		// Mid-range Android (6GB RAM): CSM shadows 1 cascade, 720p/1080p, foliage 0.3
		GEngine->Exec(GetWorld(), TEXT("r.Nanite 0"));
		GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Shadow.Virtual.Enable 0"));
		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Shadow.CSM.MaxCascades 1"));
		GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.55"));
		GEngine->Exec(GetWorld(), TEXT("r.Niagara.QualityLevel 0"));
		GEngine->Exec(GetWorld(), TEXT("foliage.DensityScale 0.3"));
		if (UserSettings)
		{
			UserSettings->SetOverallScalabilityLevel(0);
		}
		break;

	case ERealWorldGraphicsPreset::Mobile_Low:
	default:
		// Low-end Android (4GB RAM / 64GB storage): Disabled heavy shadows, 720p, high performance
		GEngine->Exec(GetWorld(), TEXT("r.Nanite 0"));
		GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Shadow.Virtual.Enable 0"));
		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Shadow.CSM.MaxCascades 1"));
		GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.40"));
		GEngine->Exec(GetWorld(), TEXT("r.Niagara.QualityLevel 0"));
		GEngine->Exec(GetWorld(), TEXT("foliage.DensityScale 0.15"));
		if (UserSettings)
		{
			UserSettings->SetOverallScalabilityLevel(0);
		}
		break;
	}

	// Target Frame Rate Capping
	int32 FPSCap = GetTargetFrameRate();
	GEngine->Exec(GetWorld(), *FString::Printf(TEXT("t.MaxFPS %d"), FPSCap));

	if (UserSettings)
	{
		UserSettings->SetFrameRateLimit(static_cast<float>(FPSCap));
		UserSettings->ApplySettings(false);
	}
}
