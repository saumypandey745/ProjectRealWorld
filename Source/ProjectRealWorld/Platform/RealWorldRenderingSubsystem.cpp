// ============================================================================
// RealWorldRenderingSubsystem.cpp
// Project RealWorld — Cinematic GTA5-class Rendering Pipeline
// ============================================================================

#include "Platform/RealWorldRenderingSubsystem.h"
#include "Engine/World.h"
#include "Engine/PostProcessVolume.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"
#include "RHI.h"

// ─────────────────────────────────────────────────────────────────────────────
// CVar Helpers
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldRenderingSubsystem::SetCVarInt(const TCHAR* CVarName, int32 Value)
{
    if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(CVarName))
    {
        CVar->Set(Value, ECVF_SetByCode);
    }
}

void URealWorldRenderingSubsystem::SetCVarFloat(const TCHAR* CVarName, float Value)
{
    if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(CVarName))
    {
        CVar->Set(Value, ECVF_SetByCode);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// INIT
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldRenderingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[RenderSubsystem] Initializing Project RealWorld Cinematic Pipeline"));
}

void URealWorldRenderingSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// AUTO-DETECT OPTIMAL PRESET
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldRenderingSubsystem::AutoDetectAndApplyOptimalPreset()
{
    // Check system/GPU memory to determine best preset
    const uint64 GPUMemory = FPlatformMemory::GetStats().TotalPhysical;
    const bool bIsMobile   = FPlatformProperties::RequiresCookedData() && PLATFORM_ANDROID;

    if (!bIsMobile)
    {
        if (GPUMemory >= 8ULL * 1024 * 1024 * 1024) // 8GB VRAM
        {
            ApplyQualityPreset(ECinematicQualityPreset::PCUltra);
        }
        else
        {
            ApplyQualityPreset(ECinematicQualityPreset::PCHigh);
        }
    }
    else
    {
        if (GPUMemory >= 4ULL * 1024 * 1024 * 1024)
            ApplyQualityPreset(ECinematicQualityPreset::MobileUltra);
        else if (GPUMemory >= 2ULL * 1024 * 1024 * 1024)
            ApplyQualityPreset(ECinematicQualityPreset::MobileHigh);
        else if (GPUMemory >= 1ULL * 1024 * 1024 * 1024)
            ApplyQualityPreset(ECinematicQualityPreset::MobileMedium);
        else
            ApplyQualityPreset(ECinematicQualityPreset::MobileLow);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// QUALITY PRESETS
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldRenderingSubsystem::ApplyQualityPreset(ECinematicQualityPreset Preset)
{
    CurrentPreset = Preset;

    switch (Preset)
    {
        case ECinematicQualityPreset::MobileLow:    ApplyMobileLow();    break;
        case ECinematicQualityPreset::MobileMedium: ApplyMobileMedium(); break;
        case ECinematicQualityPreset::MobileHigh:   ApplyMobileHigh();   break;
        case ECinematicQualityPreset::MobileUltra:  ApplyMobileUltra();  break;
        case ECinematicQualityPreset::PCHigh:       ApplyPCHigh();       break;
        case ECinematicQualityPreset::PCUltra:      ApplyPCUltra();      break;
    }
}

void URealWorldRenderingSubsystem::ApplyMobileLow()
{
    // Target: 30 FPS on budget Android (Snapdragon 6xx, 4GB)
    SetCVarInt(TEXT("r.ShadowQuality"),             0);
    SetCVarInt(TEXT("r.DynamicGlobalIlluminationMethod"), 0); // No Lumen
    SetCVarInt(TEXT("r.ReflectionMethod"),           0);
    SetCVarInt(TEXT("r.Shadow.Virtual.Enable"),      0);
    SetCVarInt(TEXT("r.Nanite"),                     0);
    SetCVarInt(TEXT("r.HairStrands.Enable"),         0);
    SetCVarInt(TEXT("r.MotionBlurQuality"),          0);
    SetCVarInt(TEXT("r.AmbientOcclusionLevels"),     0);
    SetCVarInt(TEXT("r.BloomQuality"),               1);
    SetCVarInt(TEXT("r.DepthOfFieldQuality"),        0);
    SetCVarFloat(TEXT("r.ViewDistanceScale"),         0.4f);
    SetCVarFloat(TEXT("r.MobileContentScaleFactor"), 0.7f);
    UE_LOG(LogTemp, Log, TEXT("[RenderSubsystem] Preset: Mobile Low"));
}

void URealWorldRenderingSubsystem::ApplyMobileMedium()
{
    // Target: 30 FPS stable on Snapdragon 7xx
    SetCVarInt(TEXT("r.ShadowQuality"),             1);
    SetCVarInt(TEXT("r.DynamicGlobalIlluminationMethod"), 0);
    SetCVarInt(TEXT("r.ReflectionMethod"),           1); // SSR
    SetCVarInt(TEXT("r.Shadow.Virtual.Enable"),      0);
    SetCVarInt(TEXT("r.Nanite"),                     0);
    SetCVarInt(TEXT("r.HairStrands.Enable"),         0);
    SetCVarInt(TEXT("r.MotionBlurQuality"),          0);
    SetCVarInt(TEXT("r.AmbientOcclusionLevels"),     1);
    SetCVarInt(TEXT("r.BloomQuality"),               2);
    SetCVarInt(TEXT("r.DepthOfFieldQuality"),        0);
    SetCVarFloat(TEXT("r.ViewDistanceScale"),         0.55f);
    SetCVarFloat(TEXT("r.MobileContentScaleFactor"), 0.85f);
    UE_LOG(LogTemp, Log, TEXT("[RenderSubsystem] Preset: Mobile Medium"));
}

void URealWorldRenderingSubsystem::ApplyMobileHigh()
{
    // Target: 60 FPS on Snapdragon 8 Gen 1+
    SetCVarInt(TEXT("r.ShadowQuality"),             2);
    SetCVarInt(TEXT("r.DynamicGlobalIlluminationMethod"), 1); // Lumen (SSGI fallback)
    SetCVarInt(TEXT("r.Lumen.HardwareRayTracing"),  0);
    SetCVarInt(TEXT("r.ReflectionMethod"),           1);
    SetCVarInt(TEXT("r.Shadow.Virtual.Enable"),      1);
    SetCVarInt(TEXT("r.Nanite"),                     1);
    SetCVarInt(TEXT("r.HairStrands.Enable"),         0);
    SetCVarInt(TEXT("r.MotionBlurQuality"),          1);
    SetCVarInt(TEXT("r.AmbientOcclusionLevels"),     1);
    SetCVarInt(TEXT("r.BloomQuality"),               3);
    SetCVarInt(TEXT("r.DepthOfFieldQuality"),        1);
    SetCVarFloat(TEXT("r.ViewDistanceScale"),         0.75f);
    SetCVarFloat(TEXT("r.MobileContentScaleFactor"), 1.0f);
    // Enable mobile TSR upscaling
    SetCVarInt(TEXT("r.TemporalAA.Upscaling"),       1);
    UE_LOG(LogTemp, Log, TEXT("[RenderSubsystem] Preset: Mobile High"));
}

void URealWorldRenderingSubsystem::ApplyMobileUltra()
{
    // Flagship: Snapdragon 8 Gen 3 / Dimensity 9300
    SetCVarInt(TEXT("r.ShadowQuality"),             3);
    SetCVarInt(TEXT("r.DynamicGlobalIlluminationMethod"), 1); // Lumen
    SetCVarInt(TEXT("r.Lumen.HardwareRayTracing"),  0);
    SetCVarInt(TEXT("r.ReflectionMethod"),           1); // Lumen reflections
    SetCVarInt(TEXT("r.Shadow.Virtual.Enable"),      1);
    SetCVarInt(TEXT("r.Nanite"),                     1);
    SetCVarInt(TEXT("r.HairStrands.Enable"),         1);
    SetCVarInt(TEXT("r.MotionBlurQuality"),          1);
    SetCVarInt(TEXT("r.AmbientOcclusionLevels"),     1);
    SetCVarInt(TEXT("r.BloomQuality"),               4);
    SetCVarInt(TEXT("r.DepthOfFieldQuality"),        1);
    SetCVarFloat(TEXT("r.ViewDistanceScale"),         0.9f);
    SetCVarFloat(TEXT("r.MobileContentScaleFactor"), 1.0f);
    SetCVarInt(TEXT("r.TemporalAA.Upscaling"),       1);
    UE_LOG(LogTemp, Log, TEXT("[RenderSubsystem] Preset: Mobile Ultra"));
}

void URealWorldRenderingSubsystem::ApplyPCHigh()
{
    SetCVarInt(TEXT("r.ShadowQuality"),             3);
    SetCVarInt(TEXT("r.DynamicGlobalIlluminationMethod"), 1); // Lumen
    SetCVarInt(TEXT("r.Lumen.HardwareRayTracing"),  0);
    SetCVarInt(TEXT("r.ReflectionMethod"),           1);
    SetCVarInt(TEXT("r.Shadow.Virtual.Enable"),      1);
    SetCVarInt(TEXT("r.Nanite"),                     1);
    SetCVarInt(TEXT("r.HairStrands.Enable"),         1);
    SetCVarInt(TEXT("r.MotionBlurQuality"),          2);
    SetCVarInt(TEXT("r.AmbientOcclusionLevels"),     2);
    SetCVarInt(TEXT("r.BloomQuality"),               5);
    SetCVarInt(TEXT("r.DepthOfFieldQuality"),        2);
    SetCVarFloat(TEXT("r.ViewDistanceScale"),         1.0f);
    SetCVarInt(TEXT("r.TemporalAA.Upscaling"),       1);
    UE_LOG(LogTemp, Log, TEXT("[RenderSubsystem] Preset: PC High"));
}

void URealWorldRenderingSubsystem::ApplyPCUltra()
{
    // Full cinematic — Lumen + Hardware Ray Tracing + Hair Strands
    SetCVarInt(TEXT("r.ShadowQuality"),             4);
    SetCVarInt(TEXT("r.DynamicGlobalIlluminationMethod"), 1);
    SetCVarInt(TEXT("r.Lumen.HardwareRayTracing"),  1); // HW RT enabled
    SetCVarInt(TEXT("r.ReflectionMethod"),           1);
    SetCVarInt(TEXT("r.Lumen.Reflections.HardwareRayTracing"), 1);
    SetCVarInt(TEXT("r.Shadow.Virtual.Enable"),      1);
    SetCVarInt(TEXT("r.Nanite"),                     1);
    SetCVarInt(TEXT("r.HairStrands.Enable"),         1);
    SetCVarInt(TEXT("r.MotionBlurQuality"),          4);
    SetCVarInt(TEXT("r.AmbientOcclusionLevels"),     4);
    SetCVarInt(TEXT("r.BloomQuality"),               5);
    SetCVarInt(TEXT("r.DepthOfFieldQuality"),        4);
    SetCVarFloat(TEXT("r.ViewDistanceScale"),         1.25f);
    SetCVarInt(TEXT("r.TemporalAA.Upscaling"),       1);
    SetCVarInt(TEXT("r.TSR.ShadingRejection.Flickering"), 1);
    UE_LOG(LogTemp, Log, TEXT("[RenderSubsystem] Preset: PC Ultra (Full Cinematic)"));
}

// ─────────────────────────────────────────────────────────────────────────────
// TIME OF DAY RENDERING
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldRenderingSubsystem::ApplyTimeOfDayRenderParams(const FTimeOfDayRenderParams& Params)
{
    FindOrCreateCinematicVolume();
    ApplyParamsToVolume(Params);
}

void URealWorldRenderingSubsystem::BlendToTimeOfDayParams(const FTimeOfDayRenderParams& TargetParams,
                                                           float InBlendDuration)
{
    // Store blend state and tick via world tick (simplified — use TimerHandle for production)
    BlendSourceParams = BlendTargetParams;
    BlendTargetParams = TargetParams;
    BlendElapsed      = 0.0f;
    BlendDuration     = InBlendDuration;
    bIsBlending       = true;
}

void URealWorldRenderingSubsystem::ApplyParamsToVolume(const FTimeOfDayRenderParams& Params)
{
    if (!CinematicPPVolume) return;

    FPostProcessSettings& PPSettings = CinematicPPVolume->Settings;

    PPSettings.bOverride_BloomIntensity        = true;
    PPSettings.BloomIntensity                  = Params.BloomIntensity;

    PPSettings.bOverride_VignetteIntensity     = true;
    PPSettings.VignetteIntensity               = Params.VignetteIntensity;

    PPSettings.bOverride_SceneFringeIntensity  = true;
    PPSettings.SceneFringeIntensity            = Params.ChromaticAberration;
}

void URealWorldRenderingSubsystem::FindOrCreateCinematicVolume()
{
    if (CinematicPPVolume) return;

    UWorld* World = GetGameInstance()->GetWorld();
    if (!World) return;

    // Find existing global PP volume
    for (TActorIterator<APostProcessVolume> It(World); It; ++It)
    {
        if (It->bUnbound)
        {
            CinematicPPVolume = *It;
            return;
        }
    }

    // Spawn one if not found
    CinematicPPVolume = World->SpawnActor<APostProcessVolume>();
    if (CinematicPPVolume)
    {
        CinematicPPVolume->bUnbound = true;
        CinematicPPVolume->BlendWeight = 1.0f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// POST PROCESS FINE CONTROL
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldRenderingSubsystem::ApplyCinematicSettings(const FCinematicPostProcessSettings& Settings)
{
    FindOrCreateCinematicVolume();
    if (!CinematicPPVolume) return;

    ActivePostProcessSettings = Settings;
    FPostProcessSettings& PP = CinematicPPVolume->Settings;

    // Filmic tone mapping
    PP.bOverride_FilmSlope                   = true;
    PP.FilmSlope                             = Settings.FilmSlope;
    PP.bOverride_FilmToe                     = true;
    PP.FilmToe                               = Settings.FilmToe;
    PP.bOverride_FilmShoulder                = true;
    PP.FilmShoulder                          = Settings.FilmShoulder;
    PP.bOverride_FilmWhiteClip               = true;
    PP.FilmWhiteClip                         = Settings.FilmWhiteClip;

    // Color grading
    PP.bOverride_ColorSaturation             = true;
    PP.ColorSaturation                       = Settings.ColorSaturation;
    PP.bOverride_ColorContrast               = true;
    PP.ColorContrast                         = Settings.ColorContrast;
    PP.bOverride_ColorGamma                  = true;
    PP.ColorGamma                            = Settings.ColorGamma;

    // Bloom
    PP.bOverride_BloomIntensity              = true;
    PP.BloomIntensity                        = Settings.BloomIntensity;
    PP.bOverride_BloomThreshold              = true;
    PP.BloomThreshold                        = Settings.BloomThreshold;

    // DOF
    PP.bOverride_DepthOfFieldFocalDistance   = true;
    PP.DepthOfFieldFocalDistance             = Settings.DOFFocalDistance;
    PP.bOverride_DepthOfFieldFstop           = true;
    PP.DepthOfFieldFstop                     = Settings.DOFAperture;
    PP.bOverride_DepthOfFieldFocalRegion     = true;
    PP.DepthOfFieldFocalRegion               = Settings.DOFFocalLength;

    // Vignette
    PP.bOverride_VignetteIntensity           = true;
    PP.VignetteIntensity                     = Settings.VignetteIntensity;

    // Chromatic Aberration
    PP.bOverride_SceneFringeIntensity        = true;
    PP.SceneFringeIntensity                  = Settings.ChromaticAberration;

    // Motion Blur
    PP.bOverride_MotionBlurAmount            = true;
    PP.MotionBlurAmount                      = Settings.MotionBlurAmount;
    PP.bOverride_MotionBlurMax               = true;
    PP.MotionBlurMax                         = Settings.MotionBlurMax;

    // Eye Adaptation
    PP.bOverride_AutoExposureLowPercent      = true;
    PP.AutoExposureLowPercent                = Settings.AutoExposureLowPercent;
    PP.bOverride_AutoExposureHighPercent     = true;
    PP.AutoExposureHighPercent               = Settings.AutoExposureHighPercent;
    PP.bOverride_AutoExposureMinBrightness   = true;
    PP.AutoExposureMinBrightness             = Settings.AutoExposureMinBrightness;
    PP.bOverride_AutoExposureMaxBrightness   = true;
    PP.AutoExposureMaxBrightness             = Settings.AutoExposureMaxBrightness;
    PP.bOverride_AutoExposureSpeedUp         = true;
    PP.AutoExposureSpeedUp                   = Settings.AutoExposureSpeedUp;
    PP.bOverride_AutoExposureSpeedDown       = true;
    PP.AutoExposureSpeedDown                 = Settings.AutoExposureSpeedDown;
}

void URealWorldRenderingSubsystem::SetDOFFocusDistance(float Distance)
{
    FindOrCreateCinematicVolume();
    if (CinematicPPVolume)
    {
        CinematicPPVolume->Settings.bOverride_DepthOfFieldFocalDistance = true;
        CinematicPPVolume->Settings.DepthOfFieldFocalDistance = Distance;
    }
}

void URealWorldRenderingSubsystem::EnableCinematicDOF(bool bEnable)
{
    SetCVarInt(TEXT("r.DepthOfFieldQuality"), bEnable ? 2 : 0);
}

void URealWorldRenderingSubsystem::SetChromaticAberration(float Amount)
{
    FindOrCreateCinematicVolume();
    if (CinematicPPVolume)
    {
        CinematicPPVolume->Settings.bOverride_SceneFringeIntensity = true;
        CinematicPPVolume->Settings.SceneFringeIntensity = Amount;
    }
}

void URealWorldRenderingSubsystem::SetVignetteIntensity(float Intensity)
{
    FindOrCreateCinematicVolume();
    if (CinematicPPVolume)
    {
        CinematicPPVolume->Settings.bOverride_VignetteIntensity = true;
        CinematicPPVolume->Settings.VignetteIntensity = Intensity;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// DYNAMIC EFFECTS
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldRenderingSubsystem::TriggerRainRenderingMode(bool bEnable)
{
    FindOrCreateCinematicVolume();
    if (!CinematicPPVolume) return;

    if (bEnable)
    {
        // Reduce contrast, add slight blue tint, increase chromatic aberration
        CinematicPPVolume->Settings.bOverride_ColorSaturation = true;
        CinematicPPVolume->Settings.ColorSaturation = FVector4(0.95f, 0.98f, 1.05f, 1.0f);

        CinematicPPVolume->Settings.bOverride_SceneFringeIntensity = true;
        CinematicPPVolume->Settings.SceneFringeIntensity = 0.5f;

        // Enable screen-space wet surface puddle reflections
        SetCVarInt(TEXT("r.ReflectionMethod"), 1);
    }
    else
    {
        CinematicPPVolume->Settings.bOverride_ColorSaturation = false;
        CinematicPPVolume->Settings.bOverride_SceneFringeIntensity = false;
    }
}

void URealWorldRenderingSubsystem::TriggerExplosionBloom(float Intensity, float Duration)
{
    FindOrCreateCinematicVolume();
    if (!CinematicPPVolume) return;

    PreExplosionBloomIntensity = ActivePostProcessSettings.BloomIntensity;

    CinematicPPVolume->Settings.bOverride_BloomIntensity = true;
    CinematicPPVolume->Settings.BloomIntensity = Intensity;

    // Reset after duration
    UWorld* World = GetGameInstance()->GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            ExplosionBloomTimer,
            this, &URealWorldRenderingSubsystem::OnExplosionBloomEnd,
            Duration, false
        );
    }
}

void URealWorldRenderingSubsystem::OnExplosionBloomEnd()
{
    if (CinematicPPVolume)
    {
        CinematicPPVolume->Settings.BloomIntensity = PreExplosionBloomIntensity;
    }
}

void URealWorldRenderingSubsystem::TriggerNightVision(bool bEnable)
{
    FindOrCreateCinematicVolume();
    if (!CinematicPPVolume) return;

    if (bEnable)
    {
        CinematicPPVolume->Settings.bOverride_ColorSaturation = true;
        CinematicPPVolume->Settings.ColorSaturation = FVector4(0.0f, 1.8f, 0.0f, 1.0f); // Green channel

        CinematicPPVolume->Settings.bOverride_AutoExposureMinBrightness = true;
        CinematicPPVolume->Settings.AutoExposureMinBrightness = 3.0f; // Boost dark areas

        CinematicPPVolume->Settings.bOverride_VignetteIntensity = true;
        CinematicPPVolume->Settings.VignetteIntensity = 0.7f;
    }
    else
    {
        CinematicPPVolume->Settings.bOverride_ColorSaturation      = false;
        CinematicPPVolume->Settings.bOverride_AutoExposureMinBrightness = false;
        CinematicPPVolume->Settings.bOverride_VignetteIntensity    = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// LUMEN / VSM RUNTIME CONTROL
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldRenderingSubsystem::SetLumenEnabled(bool bEnabled)
{
    SetCVarInt(TEXT("r.DynamicGlobalIlluminationMethod"), bEnabled ? 1 : 0);
    SetCVarInt(TEXT("r.ReflectionMethod"),               bEnabled ? 1 : 0);
}

void URealWorldRenderingSubsystem::SetVirtualShadowMapsEnabled(bool bEnabled)
{
    SetCVarInt(TEXT("r.Shadow.Virtual.Enable"), bEnabled ? 1 : 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// MOBILE SPECIFIC
// ─────────────────────────────────────────────────────────────────────────────
void URealWorldRenderingSubsystem::SetMobileResolutionScale(float Scale)
{
    // Clamp to sensible range
    const float ClampedScale = FMath::Clamp(Scale, 0.5f, 1.5f);
    SetCVarFloat(TEXT("r.MobileContentScaleFactor"), ClampedScale);
    UE_LOG(LogTemp, Log, TEXT("[RenderSubsystem] Mobile Resolution Scale: %.2f"), ClampedScale);
}

void URealWorldRenderingSubsystem::EnableMobileFSR(bool bEnable, float Sharpness)
{
    SetCVarInt(TEXT("r.FidelityFX.FSR.Enabled"), bEnable ? 1 : 0);
    if (bEnable)
    {
        SetCVarFloat(TEXT("r.FidelityFX.FSR.Sharpness"), Sharpness);
        UE_LOG(LogTemp, Log, TEXT("[RenderSubsystem] Mobile FSR enabled (sharpness=%.2f)"), Sharpness);
    }
}
