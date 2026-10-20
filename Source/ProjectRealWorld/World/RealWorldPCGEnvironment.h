#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealWorldPCGEnvironment.generated.h"

class UInstancedStaticMeshComponent;

USTRUCT(BlueprintType)
struct FPCGExclusionZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Exclusion")
	FBox BoxBounds;
};

UCLASS()
class PROJECTREALWORLD_API ARealWorldPCGEnvironment : public AActor
{
	GENERATED_BODY()

public:
	ARealWorldPCGEnvironment();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG|Instances")
	TObjectPtr<UInstancedStaticMeshComponent> SidewalkGrassInstances;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG|Instances")
	TObjectPtr<UInstancedStaticMeshComponent> StreetClutterInstances;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG|Instances")
	TObjectPtr<UInstancedStaticMeshComponent> PlanterVegetationInstances;

	/** Fixed deterministic seed for reproducible placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Config")
	int32 GenerationSeed = 1337;

	/** Total count of grass tufts in pavement seams */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Budgets")
	int32 GrassTuftBudget = 120;

	/** Total count of street debris items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Budgets")
	int32 ClutterBudget = 45;

	/** Exclusion zones preventing spawning on road surfaces or in doorways */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Config")
	TArray<FPCGExclusionZone> ExclusionZones;

	/** Execute deterministic PCG population */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "PCG|Actions")
	void GenerateEnvironmentDressing();

	/** Test if candidate location is inside an exclusion zone */
	UFUNCTION(BlueprintPure, Category = "PCG|Validation")
	bool IsInExclusionZone(const FVector& Location) const;
};
