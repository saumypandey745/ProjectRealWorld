#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RealWorldMaterialManager.generated.h"

class UMaterialParameterCollection;
class UMaterialInstanceDynamic;

USTRUCT(BlueprintType)
struct FRealWorldPBRMaterialParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material|PBR")
	FLinearColor BaseColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material|PBR")
	float Roughness = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material|PBR")
	float Metallic = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material|PBR")
	float NormalStrength = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material|PBR")
	float AmbientOcclusion = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material|PBR")
	float DirtAccumulation = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material|PBR")
	float WetnessInfluence = 1.0f; // Multiplier responding to WeatherSubsystem RoadWetness

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material|PBR")
	float SurfaceVariation = 0.2f;
};

UENUM(BlueprintType)
enum class ECityMaterialInstanceType : uint8
{
	Asphalt UMETA(DisplayName = "Asphalt"),
	WetAsphalt UMETA(DisplayName = "Wet Asphalt"),
	Concrete UMETA(DisplayName = "Concrete"),
	Brick UMETA(DisplayName = "Brick"),
	PaintedConcrete UMETA(DisplayName = "Painted Concrete"),
	Metal UMETA(DisplayName = "Metal"),
	Glass UMETA(DisplayName = "Glass"),
	Sidewalk UMETA(DisplayName = "Sidewalk"),
	RoadMarkings UMETA(DisplayName = "Road Markings"),
	Curbs UMETA(DisplayName = "Curbs")
};

UCLASS()
class PROJECTREALWORLD_API URealWorldMaterialManager : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Get preset physical parameters for standard city materials */
	UFUNCTION(BlueprintPure, Category = "Materials")
	FRealWorldPBRMaterialParameters GetMaterialPreset(ECityMaterialInstanceType MaterialType) const;

	/** Current global road wetness dispatched to all surface shaders */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials|State")
	float GlobalWetness = 0.0f;

	/** Puddle coverage calculated from rain saturation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials|State")
	float PuddleCoverage = 0.0f;

	/** Window and building night emissive power [0.0 = daytime off, 1.0 = nighttime illuminated] */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials|State")
	float WindowEmissivePower = 0.0f;

	/** Streetlight fixture power */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials|State")
	float StreetlightPower = 0.0f;

private:
	void UpdateGlobalShaderParameters(float DeltaTime);
};
