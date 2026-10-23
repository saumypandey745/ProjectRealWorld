#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IndianCityTypes.generated.h"

// ─── Indian City District Types ───────────────────────────────────────────────
UENUM(BlueprintType)
enum class EIndianDistrict : uint8
{
	OldCity         UMETA(DisplayName = "Old City / Purani Basti"),
	ResidentialLow  UMETA(DisplayName = "Residential Low (Chawl / Colony)"),
	ResidentialMid  UMETA(DisplayName = "Residential Mid (Apartments)"),
	Commercial      UMETA(DisplayName = "Commercial / Market"),
	Industrial      UMETA(DisplayName = "Industrial Area"),
	Slum            UMETA(DisplayName = "Jhuggi / Slum"),
	RichColony      UMETA(DisplayName = "Rich Colony / Posh Area"),
	Highway         UMETA(DisplayName = "National Highway"),
	Village         UMETA(DisplayName = "Village / Gaon"),
	Forest          UMETA(DisplayName = "Jungle / Van"),
	Beach           UMETA(DisplayName = "Samudra Tatiya / Beach"),
	River           UMETA(DisplayName = "Nadi / River Bank"),
	Farm            UMETA(DisplayName = "Khet / Farmland"),
	TouristSpot     UMETA(DisplayName = "Tourist Spot (Fort / Mandir)")
};

// ─── Indian Vehicle Types ─────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EIndianVehicleType : uint8
{
	AutoRickshaw    UMETA(DisplayName = "Auto Rickshaw"),
	Tuk Tuk         UMETA(DisplayName = "Tuk Tuk"),
	Hero_Splendor   UMETA(DisplayName = "Hero Splendor / Bajaj Pulsar"),
	Maruti_800      UMETA(DisplayName = "Maruti 800"),
	Maruti_Swift    UMETA(DisplayName = "Maruti Swift / Dzire"),
	Innova          UMETA(DisplayName = "Toyota Innova Crysta"),
	Tata_Truck      UMETA(DisplayName = "Tata 407 / Truck"),
	Tractor         UMETA(DisplayName = "Kheti Tractor"),
	CityBus         UMETA(DisplayName = "DTC / MSRTC City Bus"),
	Police_Gypsy    UMETA(DisplayName = "Police Gypsy / Bolero"),
	Ambulance       UMETA(DisplayName = "Indian Ambulance"),
	Cycle           UMETA(DisplayName = "Cycle / Bicycle"),
	Boat            UMETA(DisplayName = "Fishing Boat / Naav"),
	BulockCart      UMETA(DisplayName = "Bailgadi / Bullock Cart")
};

// ─── Indian NPC Archetypes ────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EIndianNPCType : uint8
{
	ChaiWala        UMETA(DisplayName = "Chai Wala"),
	AutoDriver      UMETA(DisplayName = "Auto Driver"),
	PoliceConstable UMETA(DisplayName = "Police Constable"),
	VendorStreet    UMETA(DisplayName = "Street Vendor / Rehdi Wala"),
	Student         UMETA(DisplayName = "College Student"),
	BusinessMan     UMETA(DisplayName = "Businessman (Kurta)"),
	Worker          UMETA(DisplayName = "Factory Worker / Mazdoor"),
	Farmer          UMETA(DisplayName = "Kisan / Farmer"),
	Priest          UMETA(DisplayName = "Pandit / Priest"),
	Tourist         UMETA(DisplayName = "Tourist (Backpacker)"),
	CivilMale       UMETA(DisplayName = "Common Man"),
	CivilFemale     UMETA(DisplayName = "Common Woman (Saree/Salwar)"),
	LocalGunda      UMETA(DisplayName = "Local Gunda / Rowdy"),
	SecurityGuard   UMETA(DisplayName = "Security Guard")
};

// ─── Indian Weather Types ─────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EIndianSeason : uint8
{
	Summer          UMETA(DisplayName = "Garmi (April-June) — Scorching Heat"),
	Monsoon         UMETA(DisplayName = "Barish (July-September) — Heavy Rain"),
	PostMonsoon     UMETA(DisplayName = "Sharad (Oct-Nov) — Pleasant"),
	Winter          UMETA(DisplayName = "Sardi (Dec-Feb) — Cold/Foggy")
};

// ─── Indian Road Sign / Street Tags ──────────────────────────────────────────
UENUM(BlueprintType)
enum class EIndianStreetTag : uint8
{
	MainRoad,
	GaliLane,
	NationalHighway,
	StateHighway,
	DirtKacha,
	Railway,
	CobblestoneOldCity
};

// ─── District Config Data Asset ───────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FIndianDistrictConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	EIndianDistrict DistrictType = EIndianDistrict::ResidentialLow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	FText DisplayName = FText::FromString(TEXT("Colony"));

	// Traffic density 0-1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TrafficDensity = 0.5f;

	// Pedestrian density 0-1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PedestrianDensity = 0.5f;

	// Crime level 0-1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CrimeRate = 0.2f;

	// Ambient sound cue tag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	FName AmbientSoundTag = TEXT("City_Low");

	// Foliage density multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float FoliageDensityMultiplier = 0.3f;
};
