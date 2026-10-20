#include "Interaction/RealWorldPickup.h"
#include "Character/RealWorldCharacter.h"
#include "Core/RealWorldPlayerState.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"

ARealWorldPickup::ARealWorldPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);
	SphereCollision->InitSphereRadius(45.0f);
	SphereCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(SphereCollision);
	PickupMesh->SetCollisionProfileName(TEXT("NoCollision"));

	PickupType = EPickupType::Cash;
	Value = 250.0f;
	ItemName = FText::FromString(TEXT("Cash Bundle"));
}

FText ARealWorldPickup::GetInteractionPrompt_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	return FText::Format(FText::FromString(TEXT("Pick Up {0}")), ItemName);
}

bool ARealWorldPickup::CanInteract_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	return true;
}

void ARealWorldPickup::Interact_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	if (!InstigatorCharacter) return;

	if (APlayerController* PC = Cast<APlayerController>(InstigatorCharacter->GetController()))
	{
		if (ARealWorldPlayerState* PS = PC->GetPlayerState<ARealWorldPlayerState>())
		{
			switch (PickupType)
			{
			case EPickupType::Cash:
				PS->AddCash(Value);
				break;
			case EPickupType::Medkit:
				InstigatorCharacter->CurrentHealth = FMath::Min(InstigatorCharacter->MaxHealth, InstigatorCharacter->CurrentHealth + Value);
				break;
			default:
				break;
			}
		}
	}

	Destroy();
}
