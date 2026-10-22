// ============================================================================
// RealWorldRadioSubsystem.cpp
// Project RealWorld — GTA5-class Vehicle Radio Stations & Police Scanner Audio
// ============================================================================

#include "Audio/RealWorldRadioSubsystem.h"
#include "Kismet/GameplayStatics.h"

void URealWorldRadioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    BuildDefaultStations();
    CurrentStationIndex = 0;
    bIsRadioPlaying = false;
    RadioVolume = 0.85f;
}

void URealWorldRadioSubsystem::BuildDefaultStations()
{
    Stations.Empty();

    // 0: Radio OFF
    {
        FRadioStationInfo Station;
        Station.StationID = TEXT("Radio_Off");
        Station.StationName = FText::FromString(TEXT("Radio OFF"));
        Station.GenreDescription = FText::FromString(TEXT("Mute"));
        Station.bIsOffStation = true;
        Stations.Add(Station);
    }
    // 1: Solace Rock FM
    {
        FRadioStationInfo Station;
        Station.StationID = TEXT("Solace_Rock");
        Station.StationName = FText::FromString(TEXT("Solace Rock FM"));
        Station.GenreDescription = FText::FromString(TEXT("Classic & Alternative Rock"));
        Station.bIsOffStation = false;
        Stations.Add(Station);
    }
    // 2: West Coast Hip Hop
    {
        FRadioStationInfo Station;
        Station.StationID = TEXT("WestCoast_Rap");
        Station.StationName = FText::FromString(TEXT("West Coast Classics"));
        Station.GenreDescription = FText::FromString(TEXT("Hip-Hop & Urban Beats"));
        Station.bIsOffStation = false;
        Stations.Add(Station);
    }
    // 3: Non-Stop Pop Hits
    {
        FRadioStationInfo Station;
        Station.StationID = TEXT("NonStop_Pop");
        Station.StationName = FText::FromString(TEXT("Non-Stop Pop 98.4"));
        Station.GenreDescription = FText::FromString(TEXT("Top 40 & Dance Hits"));
        Station.bIsOffStation = false;
        Stations.Add(Station);
    }
    // 4: Synthwave Mirror Park
    {
        FRadioStationInfo Station;
        Station.StationID = TEXT("MirrorPark_Synth");
        Station.StationName = FText::FromString(TEXT("Radio Mirror Park"));
        Station.GenreDescription = FText::FromString(TEXT("Chillwave & Electro Indie"));
        Station.bIsOffStation = false;
        Stations.Add(Station);
    }
    // 5: Solace Public Talk Radio
    {
        FRadioStationInfo Station;
        Station.StationID = TEXT("Public_Talk");
        Station.StationName = FText::FromString(TEXT("Solace News & Talk"));
        Station.GenreDescription = FText::FromString(TEXT("Satirical Talk Shows & News"));
        Station.bIsOffStation = false;
        Stations.Add(Station);
    }
}

void URealWorldRadioSubsystem::SetStationIndex(int32 NewIndex)
{
    if (Stations.Num() == 0) return;

    CurrentStationIndex = FMath::Clamp(NewIndex, 0, Stations.Num() - 1);
    const FRadioStationInfo& ActiveStation = Stations[CurrentStationIndex];

    if (ActiveStation.bIsOffStation)
    {
        StopCurrentStationAudio();
    }
    else
    {
        PlayCurrentStationAudio();
    }

    OnRadioStationChanged.Broadcast(CurrentStationIndex, ActiveStation);
}

void URealWorldRadioSubsystem::NextStation()
{
    if (Stations.Num() == 0) return;
    const int32 NextIdx = (CurrentStationIndex + 1) % Stations.Num();
    SetStationIndex(NextIdx);
}

void URealWorldRadioSubsystem::PreviousStation()
{
    if (Stations.Num() == 0) return;
    const int32 PrevIdx = (CurrentStationIndex - 1 + Stations.Num()) % Stations.Num();
    SetStationIndex(PrevIdx);
}

void URealWorldRadioSubsystem::SetRadioEnabled(bool bEnabled)
{
    if (bEnabled)
    {
        if (CurrentStationIndex == 0 && Stations.Num() > 1)
        {
            CurrentStationIndex = 1; // Auto pick first music station
        }
        SetStationIndex(CurrentStationIndex);
    }
    else
    {
        SetStationIndex(0); // Radio Off
    }
}

void URealWorldRadioSubsystem::SetRadioVolume(float Volume)
{
    RadioVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

const FRadioStationInfo& URealWorldRadioSubsystem::GetCurrentStation() const
{
    static FRadioStationInfo EmptyStation;
    if (Stations.IsValidIndex(CurrentStationIndex))
    {
        return Stations[CurrentStationIndex];
    }
    return EmptyStation;
}

void URealWorldRadioSubsystem::OnPlayerEnteredVehicle()
{
    // Turn radio on to last station or default to West Coast Classics
    if (CurrentStationIndex == 0 && Stations.Num() > 2)
    {
        CurrentStationIndex = 2;
    }
    SetStationIndex(CurrentStationIndex);
}

void URealWorldRadioSubsystem::OnPlayerExitedVehicle()
{
    StopCurrentStationAudio();
}

void URealWorldRadioSubsystem::TriggerPoliceDispatchCallout(const FString& CrimeLocation, int32 WantedLevel)
{
    FString DispatchText;
    switch (WantedLevel)
    {
        case 1:
            DispatchText = FString::Printf(TEXT("Dispatch: 10-4, units be advised, suspicious activity reported near %s."), *CrimeLocation);
            break;
        case 2:
            DispatchText = FString::Printf(TEXT("Dispatch: Attention all cars, Code 3 pursuit authorized near %s. Suspect vehicle in motion."), *CrimeLocation);
            break;
        case 3:
            DispatchText = FString::Printf(TEXT("Dispatch: All patrol units, suspect is armed and dangerous near %s. Roadblocks authorized!"), *CrimeLocation);
            break;
        case 4:
        case 5:
            DispatchText = FString::Printf(TEXT("Dispatch: Tactical SWAT teams deploying to %s! Suspect is hostile, lethal force authorized!"), *CrimeLocation);
            break;
        default:
            DispatchText = FString::Printf(TEXT("Dispatch: Scanner clear in %s sector."), *CrimeLocation);
            break;
    }

    if (PoliceChirpSound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), PoliceChirpSound);
    }

    OnPoliceScannerBroadcast.Broadcast(DispatchText);
}

void URealWorldRadioSubsystem::PlayCurrentStationAudio()
{
    bIsRadioPlaying = true;
    if (Stations.IsValidIndex(CurrentStationIndex) && Stations[CurrentStationIndex].StationAudio && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Stations[CurrentStationIndex].StationAudio, RadioVolume);
    }
}

void URealWorldRadioSubsystem::StopCurrentStationAudio()
{
    bIsRadioPlaying = false;
}
