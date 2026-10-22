#pragma once

#include "CoreMinimal.h"
#include "RealWorldScalabilityTypes.generated.h"

UENUM(BlueprintType)
enum class ERealWorldPlatformType : uint8
{
	WindowsPC UMETA(DisplayName = "Windows PC"),
	LinuxPC UMETA(DisplayName = "Linux PC"),
	Android UMETA(DisplayName = "Android"),
	IOS UMETA(DisplayName = "iOS / iPadOS"),
	EditorEnvironment UMETA(DisplayName = "Unreal Editor")
};

UENUM(BlueprintType)
enum class EPlatformPerformanceTier : uint8
{
	Low UMETA(DisplayName = "Low Tier"),
	Medium UMETA(DisplayName = "Medium Tier"),
	High UMETA(DisplayName = "High Tier"),
	Ultra UMETA(DisplayName = "Ultra Tier"),
	MobileLow UMETA(DisplayName = "Mobile Low"),
	MobileMedium UMETA(DisplayName = "Mobile Medium"),
	MobileHigh UMETA(DisplayName = "Mobile High")
};

UENUM(BlueprintType)
enum class ERealWorldGraphicsPreset : uint8
{
	Auto UMETA(DisplayName = "Auto Detect"),
	PC_Low UMETA(DisplayName = "PC Low"),
	PC_Medium UMETA(DisplayName = "PC Medium"),
	PC_High UMETA(DisplayName = "PC High"),
	PC_Ultra UMETA(DisplayName = "PC Ultra"),
	Mobile_Low UMETA(DisplayName = "Mobile Low"),
	Mobile_Medium UMETA(DisplayName = "Mobile Medium"),
	Mobile_High UMETA(DisplayName = "Mobile High")
};

UENUM(BlueprintType)
enum class EBatteryThermalMode : uint8
{
	Quality UMETA(DisplayName = "Quality"),
	Balanced UMETA(DisplayName = "Balanced"),
	Performance UMETA(DisplayName = "Performance"),
	BatterySaver UMETA(DisplayName = "Battery Saver")
};

UENUM(BlueprintType)
enum class ERealWorldResolutionMode : uint8
{
	Res_720p UMETA(DisplayName = "HD (720p) - High Performance"),
	Res_1080p UMETA(DisplayName = "Full HD (1080p) - Balanced AAA"),
	Res_2K UMETA(DisplayName = "2K (1440p) - Flagship Visuals"),
	Res_4K UMETA(DisplayName = "4K (2160p) - Ultra Photorealism")
};

USTRUCT(BlueprintType)
struct FPlatformHardwareInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	ERealWorldPlatformType PlatformType = ERealWorldPlatformType::WindowsPC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	EPlatformPerformanceTier PerformanceTier = EPlatformPerformanceTier::High;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	FString GPUDescription = TEXT("Unknown GPU");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	int32 PhysicalCores = 4;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	int32 TotalSystemMemoryMB = 8192;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	bool bSupportsTouch = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	bool bSupportsKeyboardMouse = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	bool bSupportsGamepad = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	FIntPoint ScreenResolution = FIntPoint(1920, 1080);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	float AspectRatio = 1.777f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	ERealWorldGraphicsPreset RecommendedPreset = ERealWorldGraphicsPreset::PC_High;
};

USTRUCT(BlueprintType)
struct FCustomGraphicsSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	ERealWorldGraphicsPreset ActivePreset = ERealWorldGraphicsPreset::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	ERealWorldResolutionMode ResolutionMode = ERealWorldResolutionMode::Res_1080p;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 ViewDistanceQuality = 2; // 0-3 (or Cine)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 AntiAliasingQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 ShadowQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 PostProcessQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 TextureQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 EffectsQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 FoliageQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 GlobalIlluminationQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 ReflectionQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float ResolutionScale = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 TargetFPS = 60; // 30, 60, 90, 120, 0 (unlimited)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float NPCDensityMultiplier = 1.0f; // 0.25 - 1.5

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float TrafficDensityMultiplier = 1.0f; // 0.25 - 1.5

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableVSync = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EBatteryThermalMode BatteryMode = EBatteryThermalMode::Balanced;
};
