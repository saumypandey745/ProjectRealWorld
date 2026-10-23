#pragma once

#include "CoreMinimal.h"
#include "AI/RealWorldNPCCharacter.h"
#include "World/IndianCityTypes.h"
#include "IndianMetaHumanNPC.generated.h"

class USoundCue;

UENUM(BlueprintType)
enum class EIndianAttireStyle : uint8
{
	KurtaPajama      UMETA(DisplayName = "Kurta Pajama — Traditional Male"),
	SareeTraditional UMETA(DisplayName = "Saree — Traditional Female"),
	SalwarKameez     UMETA(DisplayName = "Salwar Kameez — Casual Female"),
	KhakiPolice      UMETA(DisplayName = "Khaki Uniform — Police / Traffic"),
	ChaiWalaApron    UMETA(DisplayName = "Vest & Gamchha — Chaiwala / Worker"),
	DhotiGamchha     UMETA(DisplayName = "Dhoti & Turban — Kisan / Farmer"),
	JeansTShirt      UMETA(DisplayName = "Jeans & T-Shirt — Modern College Youth")
};

/**
 * AIndianMetaHumanNPC
 * Week 3 Core Deliverable:
 * Indian MetaHuman Realistic Character System.
 * Supports:
 * - 7 authentic Indian attire archetypes (Kurta, Saree, Khaki Police, Chaiwala, Kisan, etc.).
 * - Modular MetaHuman head, hair, mustache, clothing and Kolhapuri chappal/shoe binding.
 * - Contextual Hindi & Indian dialogue lines with subtitle UI.
 * - Dynamic store interaction (e.g. Chaiwala selling cutting chai for energy, Street food for HP).
 * - Police challan / traffic checkpoint interaction for player without helmet or in stolen auto.
 */
UCLASS()
class PROJECTREALWORLD_API AIndianMetaHumanNPC : public ARealWorldNPCCharacter
{
	GENERATED_BODY()

public:
	AIndianMetaHumanNPC();

	virtual void BeginPlay() override;

	// ─── Indian Archetype & Attire ────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IndianNPC|Identity")
	EIndianNPCType IndianType = EIndianNPCType::CivilMale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IndianNPC|Attire")
	EIndianAttireStyle AttireStyle = EIndianAttireStyle::KurtaPajama;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IndianNPC|Identity")
	FString NPCName = TEXT("Ramesh Kumar");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IndianNPC|Identity")
	FString NativeCity = TEXT("Mumbai");

	// ─── Dialogue & Audio ─────────────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IndianNPC|Dialogue")
	FString LastSpokenDialogue = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IndianNPC|Audio")
	TObjectPtr<USoundCue> Snd_VoiceLine;

	// ─── Interaction & Gameplay ───────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "IndianNPC|Dialogue")
	void SpeakContextualGreeting();

	UFUNCTION(BlueprintCallable, Category = "IndianNPC|Interaction")
	bool BuyCuttingChai(class ARealWorldCharacter* InPlayer);

	UFUNCTION(BlueprintCallable, Category = "IndianNPC|Interaction")
	bool BuyStreetFood(class ARealWorldCharacter* InPlayer);

	UFUNCTION(BlueprintCallable, Category = "IndianNPC|Police")
	void CheckPlayerTrafficViolation(class ARealWorldCharacter* InPlayer);

	// IInteractableInterface override
	virtual FText GetInteractionPrompt_Implementation(ARealWorldCharacter* InstigatorCharacter) override;
	virtual void Interact_Implementation(ARealWorldCharacter* InstigatorCharacter) override;

protected:
	void ConfigureIndianAttireMesh();
	void SelectRandomIndianName();
};
