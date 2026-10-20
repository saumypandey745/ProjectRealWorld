#include "Platform/RealWorldPlatformSubsystem.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"
#include "RHI.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"

void URealWorldPlatformSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DetectPlatformAndCapabilities();

	// Automatically apply initial recommended graphics preset
	ApplyGraphicsPreset(HardwareInfo.RecommendedPreset);
}

void URealWorldPlatformSubsystem::DetectPlatformAndCapabilities()
{
	if (bHasInitializedHardware) return;

	// 1. Identify Target Operating System
#if WITH_EDITOR
	HardwareInfo.PlatformType = ERealWorldPlatformType::EditorEnvironment;
	HardwareInfo.bSupportsTouch = false;
	HardwareInfo.bSupportsKeyboardMouse = true;
	HardwareInfo.bSupportsGamepad = true;
#elif PLATFORM_WINDOWS
	HardwareInfo.PlatformType = ERealWorldPlatformType::WindowsPC;
	HardwareInfo.bSupportsTouch = false;
	HardwareInfo.bSupportsKeyboardMouse = true;
	HardwareInfo.bSupportsGamepad = true;
#elif PLATFORM_LINUX
	HardwareInfo.PlatformType = ERealWorldPlatformType::LinuxPC;
	HardwareInfo.bSupportsTouch = false;
	HardwareInfo.bSupportsKeyboardMouse = true;
	HardwareInfo.bSupportsGamepad = true;
#elif PLATFORM_ANDROID
	HardwareInfo.PlatformType = ERealWorldPlatformType::Android;
	HardwareInfo.bSupportsTouch = true;
	HardwareInfo.bSupportsKeyboardMouse = false;
	HardwareInfo.bSupportsGamepad = true; // Bluetooth gamepads supported
#elif PLATFORM_IOS
	HardwareInfo.PlatformType = ERealWorldPlatformType::IOS;
	HardwareInfo.bSupportsTouch = true;
	HardwareInfo.bSupportsKeyboardMouse = false;
	HardwareInfo.bSupportsGamepad = true;
#else
	HardwareInfo.PlatformType = ERealWorldPlatformType::WindowsPC;
	HardwareInfo.bSupportsTouch = false;
	HardwareInfo.bSupportsKeyboardMouse = true;
	HardwareInfo.bSupportsGamepad = true;
#endif

	// 2. Hardware Query
	HardwareInfo.PhysicalCores = FPlatformMisc::NumberOfCores();
	
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	HardwareInfo.TotalSystemMemoryMB = static_cast<int32>(MemStats.TotalPhysical / (1024 * 1024));

	HardwareInfo.GPUDescription = GRHIAdapterName.IsEmpty() ? TEXT("Generic GPU Device") : GRHIAdapterName;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(HardwareInfo.ScreenResolution);
		if (HardwareInfo.ScreenResolution.Y > 0)
		{
			HardwareInfo.AspectRatio = static_cast<float>(HardwareInfo.ScreenResolution.X) / static_cast<float>(HardwareInfo.ScreenResolution.Y);
		}
	}
	else
	{
		HardwareInfo.ScreenResolution = FIntPoint(1920, 1080);
		HardwareInfo.AspectRatio = 1.777f;
	}

	// 3. Determine Performance Tier & Baseline Preset
	HardwareInfo.PerformanceTier = DeterminePerformanceTier();
	HardwareInfo.RecommendedPreset = DetermineRecommendedPreset(HardwareInfo.PerformanceTier);
	ActiveGraphicsPreset = HardwareInfo.RecommendedPreset;

	bHasInitializedHardware = true;
}

EPlatformPerformanceTier URealWorldPlatformSubsystem::DeterminePerformanceTier() const
{
	const bool bIsMobile = (HardwareInfo.PlatformType == ERealWorldPlatformType::Android || HardwareInfo.PlatformType == ERealWorldPlatformType::IOS);

	if (bIsMobile)
	{
		// Mobile Tiering based on RAM and CPU core count
		if (HardwareInfo.TotalSystemMemoryMB >= 7500 && HardwareInfo.PhysicalCores >= 8)
		{
			return EPlatformPerformanceTier::MobileHigh; // Flagship (Snapdragon 8 Gen 2/3, Apple A16/A17/M-series)
		}
		else if (HardwareInfo.TotalSystemMemoryMB >= 5500 && HardwareInfo.PhysicalCores >= 6)
		{
			return EPlatformPerformanceTier::MobileMedium; // Mid-range
		}
		return EPlatformPerformanceTier::MobileLow; // Budget entry
	}
	else
	{
		// PC / Editor Tiering based on RAM, Core count, and GPU capabilities
		if (HardwareInfo.TotalSystemMemoryMB >= 28000 && HardwareInfo.PhysicalCores >= 8)
		{
			return EPlatformPerformanceTier::Ultra; // Enthusiast PC (RTX 4080/4090 class)
		}
		else if (HardwareInfo.TotalSystemMemoryMB >= 14000 && HardwareInfo.PhysicalCores >= 6)
		{
			return EPlatformPerformanceTier::High; // Mainstream Gaming PC (RTX 3060/4060 class)
		}
		else if (HardwareInfo.TotalSystemMemoryMB >= 7500)
		{
			return EPlatformPerformanceTier::Medium; // Entry PC
		}
		return EPlatformPerformanceTier::Low; // Intel HD / Legacy fallback
	}
}

ERealWorldGraphicsPreset URealWorldPlatformSubsystem::DetermineRecommendedPreset(EPlatformPerformanceTier Tier) const
{
	switch (Tier)
	{
	case EPlatformPerformanceTier::Ultra:
		return ERealWorldGraphicsPreset::PC_Ultra;
	case EPlatformPerformanceTier::High:
		return ERealWorldGraphicsPreset::PC_High;
	case EPlatformPerformanceTier::Medium:
		return ERealWorldGraphicsPreset::PC_Medium;
	case EPlatformPerformanceTier::Low:
		return ERealWorldGraphicsPreset::PC_Low;
	case EPlatformPerformanceTier::MobileHigh:
		return ERealWorldGraphicsPreset::Mobile_High;
	case EPlatformPerformanceTier::MobileMedium:
		return ERealWorldGraphicsPreset::Mobile_Medium;
	case EPlatformPerformanceTier::MobileLow:
		return ERealWorldGraphicsPreset::Mobile_Low;
	default:
		return ERealWorldGraphicsPreset::PC_High;
	}
}

bool URealWorldPlatformSubsystem::IsMobilePlatform() const
{
	return HardwareInfo.PlatformType == ERealWorldPlatformType::Android || HardwareInfo.PlatformType == ERealWorldPlatformType::IOS;
}

bool URealWorldPlatformSubsystem::IsPCOrEditor() const
{
	return HardwareInfo.PlatformType == ERealWorldPlatformType::WindowsPC ||
	       HardwareInfo.PlatformType == ERealWorldPlatformType::LinuxPC ||
	       HardwareInfo.PlatformType == ERealWorldPlatformType::EditorEnvironment;
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
	int32 BaseCount = 24;

	switch (ActiveGraphicsPreset)
	{
	case ERealWorldGraphicsPreset::PC_Ultra:
		BaseCount = 32;
		break;
	case ERealWorldGraphicsPreset::PC_High:
		BaseCount = 24;
		break;
	case ERealWorldGraphicsPreset::PC_Medium:
		BaseCount = 16;
		break;
	case ERealWorldGraphicsPreset::PC_Low:
		BaseCount = 10;
		break;
	case ERealWorldGraphicsPreset::Mobile_High:
		BaseCount = 14;
		break;
	case ERealWorldGraphicsPreset::Mobile_Medium:
		BaseCount = 8;
		break;
	case ERealWorldGraphicsPreset::Mobile_Low:
		BaseCount = 4;
		break;
	default:
		BaseCount = 20;
		break;
	}

	// Apply battery saver penalty if thermal throttling is active
	if (ActiveBatteryMode == EBatteryThermalMode::BatterySaver)
	{
		BaseCount = FMath::Max(4, FMath::RoundToInt(BaseCount * 0.6f));
	}

	return BaseCount;
}

int32 URealWorldPlatformSubsystem::GetScaledTrafficDensity() const
{
	int32 BaseVehicles = 14;

	switch (ActiveGraphicsPreset)
	{
	case ERealWorldGraphicsPreset::PC_Ultra:
		BaseVehicles = 30;
		break;
	case ERealWorldGraphicsPreset::PC_High:
		BaseVehicles = 20;
		break;
	case ERealWorldGraphicsPreset::PC_Medium:
		BaseVehicles = 12;
		break;
	case ERealWorldGraphicsPreset::PC_Low:
		BaseVehicles = 6;
		break;
	case ERealWorldGraphicsPreset::Mobile_High:
		BaseVehicles = 8;
		break;
	case ERealWorldGraphicsPreset::Mobile_Medium:
		BaseVehicles = 4;
		break;
	case ERealWorldGraphicsPreset::Mobile_Low:
		BaseVehicles = 2;
		break;
	default:
		BaseVehicles = 12;
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
	case ERealWorldGraphicsPreset::PC_Ultra: return 1.5f;
	case ERealWorldGraphicsPreset::PC_High: return 1.0f;
	case ERealWorldGraphicsPreset::PC_Medium: return 0.8f;
	case ERealWorldGraphicsPreset::PC_Low: return 0.6f;
	case ERealWorldGraphicsPreset::Mobile_High: return 0.75f;
	case ERealWorldGraphicsPreset::Mobile_Medium: return 0.55f;
	case ERealWorldGraphicsPreset::Mobile_Low: return 0.40f;
	default: return 1.0f;
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
	case ERealWorldGraphicsPreset::PC_Ultra:
	case ERealWorldGraphicsPreset::PC_High:
		return 60;
	case ERealWorldGraphicsPreset::PC_Medium:
	case ERealWorldGraphicsPreset::PC_Low:
		return 60;
	case ERealWorldGraphicsPreset::Mobile_High:
		return 60;
	case ERealWorldGraphicsPreset::Mobile_Medium:
	case ERealWorldGraphicsPreset::Mobile_Low:
		return 30;
	default:
		return 60;
	}
}

void URealWorldPlatformSubsystem::ApplyScalabilityCvars(ERealWorldGraphicsPreset Preset, EBatteryThermalMode ThermalMode)
{
	if (!GEngine) return;

	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();

	switch (Preset)
	{
	case ERealWorldGraphicsPreset::PC_Ultra:
		GEngine->Exec(GetWorld(), TEXT("r.Nanite 1"));
		GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Lumen.HardwareRayTracing 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Shadow.Virtual.Enable 1"));
		GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 1.5"));
		GEngine->Exec(GetWorld(), TEXT("r.Niagara.QualityLevel 3"));
		if (UserSettings)
		{
			UserSettings->SetOverallScalabilityLevel(3); // Epic/Cine
		}
		break;

	case ERealWorldGraphicsPreset::PC_High:
		GEngine->Exec(GetWorld(), TEXT("r.Nanite 1"));
		GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Lumen.HardwareRayTracing 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Shadow.Virtual.Enable 1"));
		GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 1.0"));
		GEngine->Exec(GetWorld(), TEXT("r.Niagara.QualityLevel 2"));
		if (UserSettings)
		{
			UserSettings->SetOverallScalabilityLevel(2); // High
		}
		break;

	case ERealWorldGraphicsPreset::PC_Medium:
		GEngine->Exec(GetWorld(), TEXT("r.Nanite 1"));
		GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Shadow.Virtual.Enable 1"));
		GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.8"));
		GEngine->Exec(GetWorld(), TEXT("r.Niagara.QualityLevel 1"));
		if (UserSettings)
		{
			UserSettings->SetOverallScalabilityLevel(1); // Medium
		}
		break;

	case ERealWorldGraphicsPreset::PC_Low:
		GEngine->Exec(GetWorld(), TEXT("r.Nanite 0"));
		GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Shadow.Virtual.Enable 0"));
		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.6"));
		GEngine->Exec(GetWorld(), TEXT("r.Niagara.QualityLevel 0"));
		if (UserSettings)
		{
			UserSettings->SetOverallScalabilityLevel(0); // Low
		}
		break;

	case ERealWorldGraphicsPreset::Mobile_High:
		// Mobile Flagship: Optimized forward/mobile deferred, CSM shadows, medium view distance
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
		GEngine->Exec(GetWorld(), TEXT("r.Nanite 0"));
		GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Shadow.Virtual.Enable 0"));
		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.40"));
		GEngine->Exec(GetWorld(), TEXT("r.Niagara.QualityLevel 0"));
		GEngine->Exec(GetWorld(), TEXT("foliage.DensityScale 0.15"));
		if (UserSettings)
		{
			UserSettings->SetOverallScalabilityLevel(0);
		}
		break;

	default:
		break;
	}

	// Apply Frame Rate Capping based on battery mode
	int32 FPSCap = GetTargetFrameRate();
	GEngine->Exec(GetWorld(), *FString::Printf(TEXT("t.MaxFPS %d"), FPSCap));

	if (UserSettings)
	{
		UserSettings->SetFrameRateLimit(static_cast<float>(FPSCap));
		UserSettings->ApplySettings(false);
	}
}
