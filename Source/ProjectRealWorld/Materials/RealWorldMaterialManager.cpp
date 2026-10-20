#include "Materials/RealWorldMaterialManager.h"
#include "Subsystems/WeatherSubsystem.h"
#include "Subsystems/TimeOfDaySubsystem.h"
#include "Engine/World.h"

void URealWorldMaterialManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GlobalWetness = 0.0f;
	PuddleCoverage = 0.0f;
	WindowEmissivePower = 0.0f;
	StreetlightPower = 0.0f;
}

void URealWorldMaterialManager::Tick(float DeltaTime)
{
	UpdateGlobalShaderParameters(DeltaTime);
}

TStatId URealWorldMaterialManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URealWorldMaterialManager, STATGROUP_Tickables);
}

void URealWorldMaterialManager::UpdateGlobalShaderParameters(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 1. Sync road wetness & puddles from Weather Subsystem
	if (UWeatherSubsystem* WeatherSub = World->GetSubsystem<UWeatherSubsystem>())
	{
		GlobalWetness = WeatherSub->RoadWetness;
		// Puddles begin forming once road wetness passes 35% threshold
		PuddleCoverage = FMath::Clamp((GlobalWetness - 0.35f) / 0.65f, 0.0f, 1.0f);
	}

	// 2. Sync night lighting & window emissive states from TimeOfDay Subsystem
	if (UTimeOfDaySubsystem* TimeSub = World->GetSubsystem<UTimeOfDaySubsystem>())
	{
		const float TargetPower = TimeSub->bIsNightTime ? 1.0f : 0.0f;
		StreetlightPower = FMath::FInterpTo(StreetlightPower, TargetPower, DeltaTime, 2.0f);
		WindowEmissivePower = FMath::FInterpTo(WindowEmissivePower, TargetPower * 0.85f, DeltaTime, 1.5f);
	}
}

FRealWorldPBRMaterialParameters URealWorldMaterialManager::GetMaterialPreset(ECityMaterialInstanceType MaterialType) const
{
	FRealWorldPBRMaterialParameters P;

	switch (MaterialType)
	{
	case ECityMaterialInstanceType::Asphalt:
		// Standard dry coarse asphalt
		P.BaseColor = FLinearColor(0.12f, 0.12f, 0.13f, 1.0f);
		P.Roughness = FMath::Lerp(0.82f, 0.15f, GlobalWetness); // High-specular gloss when wet
		P.Metallic = 0.0f;
		P.NormalStrength = 1.1f;
		P.AmbientOcclusion = 0.95f;
		P.DirtAccumulation = 0.18f;
		P.WetnessInfluence = 1.0f;
		P.SurfaceVariation = 0.25f;
		break;

	case ECityMaterialInstanceType::WetAsphalt:
		// Fully flooded mirror-reflecting asphalt with puddle water rippling
		P.BaseColor = FLinearColor(0.06f, 0.06f, 0.07f, 1.0f);
		P.Roughness = 0.10f;
		P.Metallic = 0.0f;
		P.NormalStrength = 0.45f;
		P.AmbientOcclusion = 1.0f;
		P.DirtAccumulation = 0.10f;
		P.WetnessInfluence = 1.0f;
		P.SurfaceVariation = 0.15f;
		break;

	case ECityMaterialInstanceType::Concrete:
		// Structural cast concrete
		P.BaseColor = FLinearColor(0.65f, 0.63f, 0.60f, 1.0f);
		P.Roughness = FMath::Lerp(0.75f, 0.35f, GlobalWetness);
		P.Metallic = 0.0f;
		P.NormalStrength = 1.0f;
		P.AmbientOcclusion = 0.9f;
		P.DirtAccumulation = 0.22f;
		P.WetnessInfluence = 0.7f;
		P.SurfaceVariation = 0.30f;
		break;

	case ECityMaterialInstanceType::Brick:
		// Weathered urban terracotta masonry
		P.BaseColor = FLinearColor(0.55f, 0.22f, 0.16f, 1.0f);
		P.Roughness = FMath::Lerp(0.85f, 0.45f, GlobalWetness);
		P.Metallic = 0.0f;
		P.NormalStrength = 1.4f;
		P.AmbientOcclusion = 0.85f;
		P.DirtAccumulation = 0.30f;
		P.WetnessInfluence = 0.6f;
		P.SurfaceVariation = 0.35f;
		break;

	case ECityMaterialInstanceType::PaintedConcrete:
		// Storefront painted stucco
		P.BaseColor = FLinearColor(0.72f, 0.70f, 0.68f, 1.0f);
		P.Roughness = 0.68f;
		P.Metallic = 0.0f;
		P.NormalStrength = 0.8f;
		P.AmbientOcclusion = 0.95f;
		P.DirtAccumulation = 0.15f;
		P.WetnessInfluence = 0.5f;
		P.SurfaceVariation = 0.20f;
		break;

	case ECityMaterialInstanceType::Metal:
		// Galvanized steel street poles, utility boxes & frames
		P.BaseColor = FLinearColor(0.85f, 0.85f, 0.88f, 1.0f);
		P.Roughness = 0.30f;
		P.Metallic = 0.95f;
		P.NormalStrength = 0.7f;
		P.AmbientOcclusion = 1.0f;
		P.DirtAccumulation = 0.12f;
		P.WetnessInfluence = 0.4f;
		P.SurfaceVariation = 0.15f;
		break;

	case ECityMaterialInstanceType::Glass:
		// Architectural tempered glass with subtle roughness, reflection, and night illumination
		P.BaseColor = FLinearColor(0.88f, 0.94f, 0.98f, 0.3f);
		P.Roughness = 0.06f; // High specular mirror reflection
		P.Metallic = 0.05f;
		P.NormalStrength = 0.15f;
		P.AmbientOcclusion = 1.0f;
		P.DirtAccumulation = 0.05f;
		P.WetnessInfluence = 0.2f;
		P.SurfaceVariation = 0.05f;
		break;

	case ECityMaterialInstanceType::Sidewalk:
		// Scored concrete sidewalk pavers with expansion joints
		P.BaseColor = FLinearColor(0.68f, 0.66f, 0.64f, 1.0f);
		P.Roughness = FMath::Lerp(0.78f, 0.38f, GlobalWetness);
		P.Metallic = 0.0f;
		P.NormalStrength = 1.1f;
		P.AmbientOcclusion = 0.9f;
		P.DirtAccumulation = 0.25f;
		P.WetnessInfluence = 0.8f;
		P.SurfaceVariation = 0.25f;
		break;

	case ECityMaterialInstanceType::RoadMarkings:
		// Retro-reflective thermoplastic paint (traffic lane dashes, crosswalk bars)
		P.BaseColor = FLinearColor(0.96f, 0.96f, 0.92f, 1.0f);
		P.Roughness = 0.65f;
		P.Metallic = 0.0f;
		P.NormalStrength = 0.5f;
		P.AmbientOcclusion = 1.0f;
		P.DirtAccumulation = 0.28f; // Tire scrub wear
		P.WetnessInfluence = 0.9f;
		P.SurfaceVariation = 0.18f;
		break;

	case ECityMaterialInstanceType::Curbs:
		// Heavy beveled curb stone with gutter dirt accumulation
		P.BaseColor = FLinearColor(0.58f, 0.56f, 0.54f, 1.0f);
		P.Roughness = FMath::Lerp(0.80f, 0.40f, GlobalWetness);
		P.Metallic = 0.0f;
		P.NormalStrength = 1.2f;
		P.AmbientOcclusion = 0.85f;
		P.DirtAccumulation = 0.45f; // Maximum gutter debris accumulation
		P.WetnessInfluence = 0.95f;
		P.SurfaceVariation = 0.30f;
		break;
	}

	return P;
}
