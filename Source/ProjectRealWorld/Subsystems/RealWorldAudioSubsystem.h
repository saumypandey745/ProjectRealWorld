#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RealWorldAudioSubsystem.generated.h"

UENUM(BlueprintType)
enum class EAudioAcousticZone : uint8
{
	ExteriorOpenAir UMETA(DisplayName = "Exterior Open Air"),
	ExteriorUrbanAlley UMETA(DisplayName = "Exterior Urban Alley"),
	InteriorSmallRoom UMETA(DisplayName = "Interior Small Room"),
	InteriorLargeHall UMETA(DisplayName = "Interior Large Hall"),
	UndergroundTunnel UMETA(DisplayName = "Underground Tunnel")
};

UCLASS()
class PROJECTREALWORLD_API URealWorldAudioSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Current acoustic zone applied to the listener */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Acoustics")
	EAudioAcousticZone CurrentAcousticZone = EAudioAcousticZone::ExteriorOpenAir;

	/** Low-pass filter frequency for interior occlusion (Hz) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Acoustics")
	float OcclusionLPFFrequency = 20000.0f;

	/** Master wetness parameter sent to MetaSounds for rain and splashing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|MetaSounds")
	float MetaSoundRainIntensity = 0.0f;

	/** Transition acoustic zone when entering or leaving buildings */
	UFUNCTION(BlueprintCallable, Category = "Audio|Acoustics")
	void SetAcousticZone(EAudioAcousticZone NewZone);

	/** Notify audio system of listener indoor/outdoor state */
	UFUNCTION(BlueprintCallable, Category = "Audio|Acoustics")
	void SetListenerInteriorOcclusion(bool bIsIndoors, float OcclusionFactor);

private:
	float TargetLPF = 20000.0f;
};
