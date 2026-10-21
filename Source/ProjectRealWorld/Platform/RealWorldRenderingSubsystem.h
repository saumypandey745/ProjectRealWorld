// ============================================================================
// RealWorldRenderingSubsystem.h
// Project RealWorld — Cinematic Rendering Pipeline Manager
// Manages: Lumen GI, Virtual Shadow Maps, TSR, DOF, tone mapping,
//          dynamic weather rendering, mobile scalability, eye adaptation
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/PostProcessVolume.h"
#include "RealWorldRenderingSubsystem.generated.h"

class UPostProcessComponent;

// ─── Cinematic Quality Presets ────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECinematicQualityPreset : uint8
{
    MobileLow       UMETA(DisplayName = "Mobile Low (30 FPS)"),
    MobileMedium    UMETA(DisplayName = "Mobile Medium"),
    MobileHigh      UMETA(DisplayName = "Mobile High (60 FPS)"),
    MobileUltra     UMETA(DisplayName = "Mobile Ultra — Flagship"),
    PCHigh          UMETA(DisplayName = "PC High"),
    PCUltra         UMETA(DisplayName = "PC Ultra — Cinematic"),
};

// ─── Time-of-Day Rendering Params ────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FTimeOfDayRenderParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyZenithColor     = FLinearColor(0.05f, 0.12f, 0.45f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyHorizonColor    = FLinearColor(0.65f, 0.38f, 0.18f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SunColor           = FLinearColor(1.0f, 0.92f, 0.78f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity              = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity                = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogInscatterColor  = FLinearColor(0.5f, 0.6f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LumenSkyLightIntensity    = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VignetteIntensity         = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BloomIntensity            = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChromaticAberration       = 0.0f;
};

// ─── Post Process Cinematic Settings ─────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCinematicPostProcessSettings
{
    GENERATED_BODY()

    // Tone mapping (filmic, GTA5-style warm grade)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FilmSlope          = 0.88f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FilmToe            = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FilmShoulder       = 0.26f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FilmWhiteClip      = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FilmBlackClip      = 0.0f;

    // Color grading
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector4 ColorSaturation = FVector4(1.05f, 1.0f, 0.95f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector4 ColorContrast   = FVector4(1.05f, 1.02f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector4 ColorGamma      = FVector4(1.0f, 0.97f, 0.95f, 1.0f);

    // Bloom
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BloomIntensity     = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BloomThreshold     = -1.0f;

    // Lens flares
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LensFlareBokehSize = 3.0f;

    // Depth of Field (Bokeh)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DOFFocalDistance   = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DOFAperture        = 4.0f;   // f/4

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DOFFocalLength     = 50.0f;  // mm

    // Vignette
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VignetteIntensity  = 0.35f;

    // Chromatic aberration (subtle)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChromaticAberration = 0.3f;

    // Motion blur
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MotionBlurAmount   = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MotionBlurMax      = 5.0f;

    // Eye adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoExposureLowPercent  = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoExposureHighPercent = 98.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoExposureMinBrightness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoExposureMaxBrightness = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoExposureSpeedUp     = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoExposureSpeedDown   = 1.0f;
};

UCLASS()
class PROJECTREALWORLD_API URealWorldRenderingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ─── Quality Presets ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Rendering|Quality")
    void ApplyQualityPreset(ECinematicQualityPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "Rendering|Quality")
    ECinematicQualityPreset GetCurrentPreset() const { return CurrentPreset; }

    UFUNCTION(BlueprintCallable, Category = "Rendering|Quality")
    void AutoDetectAndApplyOptimalPreset();

    // ─── Time of Day ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Rendering|TimeOfDay")
    void ApplyTimeOfDayRenderParams(const FTimeOfDayRenderParams& Params);

    UFUNCTION(BlueprintCallable, Category = "Rendering|TimeOfDay")
    void BlendToTimeOfDayParams(const FTimeOfDayRenderParams& TargetParams, float BlendDuration);

    // ─── Post Process ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Rendering|PostProcess")
    void ApplyCinematicSettings(const FCinematicPostProcessSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Rendering|PostProcess")
    void SetDOFFocusDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Rendering|PostProcess")
    void EnableCinematicDOF(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Rendering|PostProcess")
    void SetChromaticAberration(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Rendering|PostProcess")
    void SetVignetteIntensity(float Intensity);

    // ─── Dynamic Effects ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Rendering|Effects")
    void TriggerRainRenderingMode(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Rendering|Effects")
    void TriggerExplosionBloom(float Intensity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Rendering|Effects")
    void TriggerNightVision(bool bEnable);

    // ─── Lumen control (runtime) ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Rendering|Lumen")
    void SetLumenEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Rendering|Lumen")
    void SetVirtualShadowMapsEnabled(bool bEnabled);

    // ─── Mobile Specific ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Rendering|Mobile")
    void SetMobileResolutionScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Rendering|Mobile")
    void EnableMobileFSR(bool bEnable, float Sharpness = 0.8f);

    // ─── Current State ───────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering|State")
    ECinematicQualityPreset CurrentPreset = ECinematicQualityPreset::MobileHigh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering|State")
    FCinematicPostProcessSettings ActivePostProcessSettings;

private:
    UPROPERTY()
    TObjectPtr<APostProcessVolume> CinematicPPVolume;

    // Time of day blend state
    FTimeOfDayRenderParams BlendSourceParams;
    FTimeOfDayRenderParams BlendTargetParams;
    float BlendElapsed   = 0.0f;
    float BlendDuration  = 0.0f;
    bool  bIsBlending    = false;

    FTimerHandle ExplosionBloomTimer;
    float PreExplosionBloomIntensity = 0.4f;

    void FindOrCreateCinematicVolume();
    void ApplyParamsToVolume(const FTimeOfDayRenderParams& Params);
    void ApplyMobileLow();
    void ApplyMobileMedium();
    void ApplyMobileHigh();
    void ApplyMobileUltra();
    void ApplyPCHigh();
    void ApplyPCUltra();
    void OnExplosionBloomEnd();

    // Scalability CVar helpers
    static void SetCVarInt(const TCHAR* CVarName, int32 Value);
    static void SetCVarFloat(const TCHAR* CVarName, float Value);
};
