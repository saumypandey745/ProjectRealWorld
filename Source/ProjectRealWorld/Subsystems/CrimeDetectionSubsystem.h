#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Subsystems/PoliceWantedSubsystem.h"
#include "CrimeDetectionSubsystem.generated.h"

class ARealWorldNPCCharacter;

USTRUCT(BlueprintType)
struct FPhysicalEvidence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crime|Evidence")
	FGuid EvidenceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crime|Evidence")
	ECrimeType RelatedCrime = ECrimeType::MinorTheft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crime|Evidence")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crime|Evidence")
	float LifetimeRemaining = 180.0f; // 3 minutes standard decay

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crime|Evidence")
	bool bDiscoveredByPolice = false;
};

USTRUCT(BlueprintType)
struct FCrimeEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crime")
	FGuid CrimeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crime")
	ECrimeType CrimeType = ECrimeType::MinorTheft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crime")
	float Severity = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crime")
	FVector CrimeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crime")
	float WorldTimestamp = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crime")
	TWeakObjectPtr<AActor> Perpetrator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crime")
	float DetectionConfidence = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crime")
	TArray<TObjectPtr<ARealWorldNPCCharacter>> Witnesses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crime")
	bool bReportedToPolice = false;
};

USTRUCT(BlueprintType)
struct FWitnessReportCall
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crime|Witness")
	TWeakObjectPtr<ARealWorldNPCCharacter> Witness;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crime|Witness")
	FCrimeEvent CrimeData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crime|Witness")
	float CallDurationRemaining = 10.0f; // 8-12 seconds to dial & report
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrimeCommitted, const FCrimeEvent&, Crime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWitnessAlerted, ARealWorldNPCCharacter*, Witness, const FCrimeEvent&, Crime);

UCLASS()
class PROJECTREALWORLD_API UCrimeDetectionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Record an unlawful action in the world and notify potential witnesses */
	UFUNCTION(BlueprintCallable, Category = "Crime|System")
	void RecordCrime(ECrimeType CrimeType, FVector Location, AActor* Perpetrator, float NoiseMagnitude = 1.0f);

	/** Add physical crime scene evidence that decays over time */
	UFUNCTION(BlueprintCallable, Category = "Crime|Evidence")
	void SpawnPhysicalEvidence(ECrimeType CrimeType, FVector Location);

	UPROPERTY(BlueprintAssignable, Category = "Crime|Events")
	FOnCrimeCommitted OnCrimeCommitted;

	UPROPERTY(BlueprintAssignable, Category = "Crime|Events")
	FOnWitnessAlerted OnWitnessAlerted;

	// Active evidence records
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crime|Evidence")
	TArray<FPhysicalEvidence> ActiveEvidence;

	// Pending 911 calls from civilian witnesses
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crime|Witnesses")
	TArray<FWitnessReportCall> PendingCalls;

protected:
	bool CheckLineOfSight(const FVector& ObserverEye, const FVector& TargetPoint, AActor* IgnoredObserver) const;
	float CalculateHearingAttenuation(float RawNoise, float Distance, float RainIntensity) const;

private:
	float WorldClock = 0.0f;
};
