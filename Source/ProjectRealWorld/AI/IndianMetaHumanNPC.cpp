#include "AI/IndianMetaHumanNPC.h"
#include "Character/RealWorldCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AIndianMetaHumanNPC::AIndianMetaHumanNPC()
{
	PrimaryActorTick.bCanEverTick = true;
	IndianType = EIndianNPCType::ChaiWala;
	AttireStyle = EIndianAttireStyle::ChaiWalaApron;
	NPCName = TEXT("Chotu Chaiwala");
}

void AIndianMetaHumanNPC::BeginPlay()
{
	Super::BeginPlay();

	ConfigureIndianAttireMesh();
	SelectRandomIndianName();
}

void AIndianMetaHumanNPC::ConfigureIndianAttireMesh()
{
	switch (IndianType)
	{
	case EIndianNPCType::ChaiWala:
		AttireStyle = EIndianAttireStyle::ChaiWalaApron;
		Archetype = ENPCArchetype::Worker;
		break;
	case EIndianNPCType::PoliceConstable:
		AttireStyle = EIndianAttireStyle::KhakiPolice;
		Archetype = ENPCArchetype::AuthorityOfficer;
		break;
	case EIndianNPCType::CivilFemale:
		AttireStyle = (FMath::RandBool()) ? EIndianAttireStyle::SareeTraditional : EIndianAttireStyle::SalwarKameez;
		Archetype = ENPCArchetype::CivilianFemale;
		break;
	case EIndianNPCType::Farmer:
		AttireStyle = EIndianAttireStyle::DhotiGamchha;
		Archetype = ENPCArchetype::Elderly;
		break;
	case EIndianNPCType::LocalGunda:
		AttireStyle = EIndianAttireStyle::JeansTShirt;
		Archetype = ENPCArchetype::CivilianMale;
		break;
	default:
		AttireStyle = EIndianAttireStyle::KurtaPajama;
		Archetype = ENPCArchetype::CivilianMale;
		break;
	}
}

void AIndianMetaHumanNPC::SelectRandomIndianName()
{
	const TArray<FString> FirstNames = {
		TEXT("Ramesh"), TEXT("Suresh"), TEXT("Chotu"), TEXT("Pappu"),
		TEXT("Munna"), TEXT("Sunita"), TEXT("Pooja"), TEXT("Kisan"),
		TEXT("Inspector Shinde"), TEXT("Kishore"), TEXT("Santosh")
	};
	NPCName = FirstNames[FMath::RandRange(0, FirstNames.Num() - 1)];
}

void AIndianMetaHumanNPC::SpeakContextualGreeting()
{
	switch (IndianType)
	{
	case EIndianNPCType::ChaiWala:
		LastSpokenDialogue = TEXT("Garam cutting chai taiyyar hai bhaiya! Ek cup ho jaye? (₹10)");
		break;
	case EIndianNPCType::AutoDriver:
		LastSpokenDialogue = TEXT("Kahan chalna hai saheb? Meter se chalenge, aaram se baitho!");
		break;
	case EIndianNPCType::PoliceConstable:
		LastSpokenDialogue = TEXT("Ruko ruko! Helmet kahan hai tumhara? License dikhao!");
		break;
	case EIndianNPCType::VendorStreet:
		LastSpokenDialogue = TEXT("Garma-garam Vada Pav aur Samosa le lo saheb! Ekdum fresh!");
		break;
	case EIndianNPCType::Farmer:
		LastSpokenDialogue = TEXT("Ram Ram bhaiya! Kripa hai bhagwan ki, khet mein kaam chal raha hai.");
		break;
	case EIndianNPCType::LocalGunda:
		LastSpokenDialogue = TEXT("Ae hero! Idhar kisse pooch ke ghoom raha hai re tu?!");
		break;
	default:
		LastSpokenDialogue = TEXT("Namaste bhaiya! Sab theek-thaak?");
		break;
	}

	if (Snd_VoiceLine)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Snd_VoiceLine, GetActorLocation());
	}

	UE_LOG(LogTemp, Log, TEXT("[%s]: \"%s\""), *NPCName, *LastSpokenDialogue);
}

bool AIndianMetaHumanNPC::BuyCuttingChai(ARealWorldCharacter* InPlayer)
{
	if (!InPlayer) return false;

	// Restore player stamina to full (100%)
	InPlayer->CurrentStamina = InPlayer->MaxStamina;
	SpeakContextualGreeting();
	UE_LOG(LogTemp, Log, TEXT("[ChaiWala] Sold cutting chai! Player stamina restored to 100%%."));
	return true;
}

bool AIndianMetaHumanNPC::BuyStreetFood(ARealWorldCharacter* InPlayer)
{
	if (!InPlayer) return false;

	// Restore player health by 50 HP
	InPlayer->CurrentHealth = FMath::Clamp(InPlayer->CurrentHealth + 50.0f, 0.0f, InPlayer->MaxHealth);
	SpeakContextualGreeting();
	UE_LOG(LogTemp, Log, TEXT("[VendorStreet] Sold hot snack! Player health restored (+50 HP)."));
	return true;
}

void AIndianMetaHumanNPC::CheckPlayerTrafficViolation(ARealWorldCharacter* InPlayer)
{
	if (!InPlayer || IndianType != EIndianNPCType::PoliceConstable) return;

	SpeakContextualGreeting();
	UE_LOG(LogTemp, Warning, TEXT("[TrafficPolice] Checkpoint inspection performed on player."));
}

FText AIndianMetaHumanNPC::GetInteractionPrompt_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	switch (IndianType)
	{
	case EIndianNPCType::ChaiWala:
		return FText::FromString(TEXT("Press to Drink Cutting Chai (₹10)"));
	case EIndianNPCType::AutoDriver:
		return FText::FromString(TEXT("Press to Hire Auto Rickshaw"));
	case EIndianNPCType::VendorStreet:
		return FText::FromString(TEXT("Press to Eat Vada Pav / Snack (₹20)"));
	case EIndianNPCType::PoliceConstable:
		return FText::FromString(TEXT("Press to Talk to Police Constable"));
	default:
		return FText::FromString(TEXT("Press to Talk"));
	}
}

void AIndianMetaHumanNPC::Interact_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	if (!InstigatorCharacter) return;

	if (IndianType == EIndianNPCType::ChaiWala)
	{
		BuyCuttingChai(InstigatorCharacter);
	}
	else if (IndianType == EIndianNPCType::VendorStreet)
	{
		BuyStreetFood(InstigatorCharacter);
	}
	else if (IndianType == EIndianNPCType::PoliceConstable)
	{
		CheckPlayerTrafficViolation(InstigatorCharacter);
	}
	else
	{
		SpeakContextualGreeting();
	}
}
