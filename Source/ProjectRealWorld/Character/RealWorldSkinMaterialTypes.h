#pragma once

#include "CoreMinimal.h"
#include "RealWorldSkinMaterialTypes.generated.h"

class USubsurfaceProfile;

USTRUCT(BlueprintType)
struct FRealWorldSkinMaterialConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin|Subsurface")
	TSoftObjectPtr<USubsurfaceProfile> SubsurfaceProfile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin|Color")
	FLinearColor BaseMelaninColor = FLinearColor(0.85f, 0.70f, 0.60f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin|Roughness")
	float BaseRoughness = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin|Roughness")
	float PorosityStrength = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin|Specular")
	float DualLobeSpecularRoughness = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin|Specular")
	float CavityOcclusion = 0.85f;
};

USTRUCT(BlueprintType)
struct FRealWorldEyeMaterialConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eye|Iris")
	FLinearColor IrisColor = FLinearColor(0.18f, 0.28f, 0.42f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eye|Iris")
	float PupilScale = 1.0f; // Controlled by ambient light level

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eye|Sclera")
	FLinearColor ScleraColor = FLinearColor(0.92f, 0.90f, 0.88f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eye|Cornea")
	float CorneaRefractionIndex = 1.336f;
};
