// ============================================================================
// RealWorldRadioSubsystem.h
// Project RealWorld — GTA5-class Vehicle Radio Stations & Police Scanner Audio
// 6 curated radio stations, wheel UI data, volume ducking, and police dispatch
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Sound/SoundBase.h"
#include "RealWorldRadioSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FRadioStationInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio")
    FName StationID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio")
    FText StationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio")
    FText GenreDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio")
    TObjectPtr<USoundBase> StationAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio")
    bool bIsOffStation = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRadioStationChanged, int32, StationIndex, const FRadioStationInfo&, Station);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPoliceScannerBroadcast, const FString&, DispatchCallout);

UCLASS()
class PROJECTREALWORLD_API URealWorldRadioSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // ─── Station Controls ────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Radio|Playback")
    void SetStationIndex(int32 NewIndex);

    UFUNCTION(BlueprintCallable, Category = "Radio|Playback")
    void NextStation();

    UFUNCTION(BlueprintCallable, Category = "Radio|Playback")
    void PreviousStation();

    UFUNCTION(BlueprintCallable, Category = "Radio|Playback")
    void SetRadioEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Radio|Playback")
    void SetRadioVolume(float Volume);

    UFUNCTION(BlueprintPure, Category = "Radio|Playback")
    const FRadioStationInfo& GetCurrentStation() const;

    UFUNCTION(BlueprintPure, Category = "Radio|Playback")
    const TArray<FRadioStationInfo>& GetAllStations() const { return Stations; }

    // ─── Vehicle Integration ─────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Radio|Vehicle")
    void OnPlayerEnteredVehicle();

    UFUNCTION(BlueprintCallable, Category = "Radio|Vehicle")
    void OnPlayerExitedVehicle();

    // ─── Police Scanner Integration ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Radio|PoliceScanner")
    void TriggerPoliceDispatchCallout(const FString& CrimeLocation, int32 WantedLevel);

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Radio|Events")
    FOnRadioStationChanged OnRadioStationChanged;

    UPROPERTY(BlueprintAssignable, Category = "Radio|Events")
    FOnPoliceScannerBroadcast OnPoliceScannerBroadcast;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio|Config")
    TArray<FRadioStationInfo> Stations;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Radio|State")
    int32 CurrentStationIndex = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Radio|State")
    bool bIsRadioPlaying = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Radio|State")
    float RadioVolume = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio|PoliceScanner")
    TObjectPtr<USoundBase> PoliceChirpSound;

    void BuildDefaultStations();
    void PlayCurrentStationAudio();
    void StopCurrentStationAudio();
};
