#pragma once

#include "CoreMinimal.h"
#include "RealWorldScalabilityTypes.generated.h"

// ─── Android-Only Platform Type ───────────────────────────────────────────────
UENUM(BlueprintType)
enum class ERealWorldPlatformType : uint8
{
	Android         UMETA(DisplayName = "Android"),
	EditorEnvironment UMETA(DisplayName = "Unreal Editor (Development Only)")
};

// ─── Android Performance Tiers ────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EPlatformPerformanceTier : uint8
{
	MobileLow    UMETA(DisplayName = "Mobile Low   — 4GB RAM, 30fps (Snapdragon 695)"),
	MobileMedium UMETA(DisplayName = "Mobile Mid   — 6GB RAM, 45fps (Snapdragon 7xx)"),
	MobileHigh   UMETA(DisplayName = "Mobile High  — 8GB RAM, 60fps (Snapdragon 8 Gen1)")
};

// ─── Android Graphics Presets ─────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ERealWorldGraphicsPreset : uint8
{
	Auto         UMETA(DisplayName = "Auto Detect"),
	Mobile_Low   UMETA(DisplayName = "Low   — 30fps on 4GB RAM"),
	Mobile_Medium UMETA(DisplayName = "Medium — 45fps on 6GB RAM"),
	Mobile_High  UMETA(DisplayName = "High  — 60fps on 8GB RAM")
};

// ─── Battery / Thermal Modes ──────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EBatteryThermalMode : uint8
{
	Performance  UMETA(DisplayName = "Performance — Max FPS, more battery"),
	Balanced     UMETA(DisplayName = "Balanced    — Default recommended"),
	BatterySaver UMETA(DisplayName = "Battery Saver — Lower FPS, less heat")
};

// ─── Resolution Options ───────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ERealWorldResolutionMode : uint8
{
	Res_720p  UMETA(DisplayName = "720p  — High Performance (4GB devices)"),
	Res_1080p UMETA(DisplayName = "1080p — Balanced (default)"),
	Res_2K    UMETA(DisplayName = "2K    — Flagship devices only")
};

// ─── Hardware Info Struct ────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FPlatformHardwareInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	ERealWorldPlatformType PlatformType = ERealWorldPlatformType::Android;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	EPlatformPerformanceTier PerformanceTier = EPlatformPerformanceTier::MobileLow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	FString GPUDescription = TEXT("Android GPU");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	int32 PhysicalCores = 8;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	int32 TotalSystemMemoryMB = 4096;

	// Always true on Android
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	bool bSupportsTouch = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	FIntPoint ScreenResolution = FIntPoint(1080, 2400);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	float AspectRatio = 20.0f / 9.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	ERealWorldGraphicsPreset RecommendedPreset = ERealWorldGraphicsPreset::Mobile_Low;
};

// ─── Graphics Settings (Android only) ────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCustomGraphicsSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	ERealWorldGraphicsPreset ActivePreset = ERealWorldGraphicsPreset::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	ERealWorldResolutionMode ResolutionMode = ERealWorldResolutionMode::Res_720p;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 ShadowQuality = 1; // 0=Off, 1=Low, 2=Medium, 3=High

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 TextureQuality = 1; // 0=Low, 1=Medium, 2=High

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 EffectsQuality = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 ViewDistanceQuality = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 TargetFPS = 30; // 30 or 60

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float NPCDensityMultiplier = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float TrafficDensityMultiplier = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableVSync = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EBatteryThermalMode BatteryMode = EBatteryThermalMode::Balanced;
};
