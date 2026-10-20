#pragma once

#include "CoreMinimal.h"
#include "RealWorldFacialAnimationTypes.generated.h"

UENUM(BlueprintType)
enum class EFacialEmotionState : uint8
{
	Neutral UMETA(DisplayName = "Neutral"),
	Happy UMETA(DisplayName = "Happy"),
	Angry UMETA(DisplayName = "Angry"),
	Fear UMETA(DisplayName = "Fear"),
	Pain UMETA(DisplayName = "Pain"),
	Surprise UMETA(DisplayName = "Surprise")
};

/**
 * Standard 52 FACS / ARKit Facial Morph Target Curve Names
 * Directly maps to MetaHuman face rigs for high-fidelity procedural expressions.
 */
USTRUCT(BlueprintType)
struct FRealWorldFacialCurveWeights
{
	GENERATED_BODY()

	// Eye & Brow Region
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Eyes")
	float EyeBlinkLeft = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Eyes")
	float EyeBlinkRight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Eyes")
	float EyeWideLeft = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Eyes")
	float EyeWideRight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Brows")
	float BrowDownLeft = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Brows")
	float BrowDownRight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Brows")
	float BrowInnerUp = 0.0f;

	// Mouth & Jaw Region
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Mouth")
	float JawOpen = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Mouth")
	float MouthSmileLeft = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Mouth")
	float MouthSmileRight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Mouth")
	float MouthFrownLeft = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Mouth")
	float MouthFrownRight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FacialCurves|Mouth")
	float MouthPucker = 0.0f;
};
