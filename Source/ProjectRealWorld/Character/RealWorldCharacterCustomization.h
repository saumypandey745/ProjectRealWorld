#pragma once

#include "CoreMinimal.h"
#include "RealWorldCharacterCustomization.generated.h"

class USkeletalMesh;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EBodyType : uint8
{
	MaleAverage UMETA(DisplayName = "Male Average"),
	MaleAthletic UMETA(DisplayName = "Male Athletic"),
	FemaleAverage UMETA(DisplayName = "Female Average"),
	FemaleAthletic UMETA(DisplayName = "Female Athletic")
};

USTRUCT(BlueprintType)
struct FRealWorldClothingSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
	TSoftObjectPtr<USkeletalMesh> TorsoMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
	TSoftObjectPtr<USkeletalMesh> LegsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
	TSoftObjectPtr<USkeletalMesh> FeetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
	TSoftObjectPtr<USkeletalMesh> AccessoriesMesh;
};

USTRUCT(BlueprintType)
struct FRealWorldCharacterCustomization
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Body")
	EBodyType BodyType = EBodyType::MaleAverage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Face")
	TSoftObjectPtr<USkeletalMesh> HeadMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Hair")
	TSoftObjectPtr<USkeletalMesh> HairMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Clothing")
	FRealWorldClothingSet Clothing;

	// Tint parameters applied dynamically via dynamic material instances
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Colors")
	FLinearColor SkinTone = FLinearColor(0.85f, 0.70f, 0.60f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Colors")
	FLinearColor HairColor = FLinearColor(0.08f, 0.05f, 0.03f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Colors")
	FLinearColor EyeColor = FLinearColor(0.15f, 0.25f, 0.35f, 1.0f);
};
