#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WeatherSubsystem.generated.h"

UENUM(BlueprintType)
enum class EWeatherType : uint8
{
	Clear UMETA(DisplayName = "Clear"),
	Cloudy UMETA(DisplayName = "Cloudy"),
	Rain UMETA(DisplayName = "Rain"),
	HeavyRain UMETA(DisplayName = "Heavy Rain"),
	Storm UMETA(DisplayName = "Storm"),
	Fog UMETA(DisplayName = "Fog")
};

USTRUCT(BlueprintType)
struct FWeatherProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float RainIntensity = 0.0f; // 0.0 to 1.0

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float CloudCover = 0.1f; // 0.0 to 1.0

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float FogDensity = 0.0001f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float WindSpeed = 5.0f; // km/h

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float VisibilityRange = 50000.0f; // Centimeters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float RoadFrictionMultiplier = 1.0f; // 1.0 dry, 0.65 wet/slick
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged, EWeatherType, NewWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoadWetnessUpdated, float, WetnessPercent);

UCLASS()
class PROJECTREALWORLD_API UWeatherSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Active target weather */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather|State")
	EWeatherType CurrentWeather = EWeatherType::Clear;

	/** Interpolated current properties applied to materials, post-process, and audio */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather|State")
	FWeatherProperties CurrentProperties;

	/** Road surface wetness [0.0 = bone dry, 1.0 = saturated puddles] */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather|Physics")
	float RoadWetness = 0.0f;

	/** Transition duration between states in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Config")
	float TransitionDuration = 30.0f;

	/** Request weather transition */
	UFUNCTION(BlueprintCallable, Category = "Weather|Control")
	void SetTargetWeather(EWeatherType NewWeather);

	/** Get preset properties for a given weather type */
	UFUNCTION(BlueprintPure, Category = "Weather|Config")
	FWeatherProperties GetPresetForWeather(EWeatherType Weather) const;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Weather|Events")
	FOnWeatherChanged OnWeatherChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weather|Events")
	FOnRoadWetnessUpdated OnRoadWetnessUpdated;

private:
	FWeatherProperties TargetProperties;
	float TransitionAlpha = 1.0f;
};
