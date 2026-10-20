#include "Interaction/RealWorldStoreItem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Character/RealWorldCharacter.h"
#include "Core/RealWorldPlayerState.h"
#include "UI/RealWorldHUD.h"
#include "Kismet/GameplayStatics.h"

ARealWorldStoreItem::ARealWorldStoreItem()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;
	ItemMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(120.0f);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	ItemID = FName(TEXT("ITEM_EnergyDrink"));
	ItemName = FText::FromString(TEXT("Nitro Energy Drink"));
	ItemDescription = FText::FromString(TEXT("Replenishes stamina and revives energy."));
	Price = 4.50f;
	bInfiniteStock = true;
	bIsPurchased = false;
}

void ARealWorldStoreItem::BeginPlay()
{
	Super::BeginPlay();
}

FText ARealWorldStoreItem::GetInteractionPrompt_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	if (bIsPurchased)
	{
		return FText::FromString(TEXT("Item Out of Stock"));
	}

	return FText::FromString(FString::Printf(TEXT("Purchase %s ($%.2f)"), *ItemName.ToString(), Price));
}

bool ARealWorldStoreItem::CanInteract_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	if (bIsPurchased || !InstigatorCharacter)
	{
		return false;
	}

	if (ARealWorldPlayerState* PS = InstigatorCharacter->GetPlayerState<ARealWorldPlayerState>())
	{
		return PS->CanAfford(Price);
	}

	return false;
}

void ARealWorldStoreItem::Interact_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	if (!InstigatorCharacter || bIsPurchased)
	{
		return;
	}

	ARealWorldPlayerState* PS = InstigatorCharacter->GetPlayerState<ARealWorldPlayerState>();
	if (!PS)
	{
		return;
	}

	FText PurchaseDesc = FText::FromString(FString::Printf(TEXT("Store Purchase: %s"), *ItemName.ToString()));
	if (PS->SpendCash(Price, PurchaseDesc, EEconomyTransactionType::StorePurchase))
	{
		PS->AddInventoryItem(ItemID);
		OnStoreItemPurchased.Broadcast(ItemID, Price, InstigatorCharacter);

		if (APlayerController* PC = Cast<APlayerController>(InstigatorCharacter->GetController()))
		{
			if (ARealWorldHUD* HUD = Cast<ARealWorldHUD>(PC->GetHUD()))
			{
				FText Feedback = FText::FromString(FString::Printf(TEXT("Purchased %s (-$%.2f)"), *ItemName.ToString(), Price));
				HUD->SetInteractionPrompt(Feedback);
			}
		}

		if (bInfiniteStock)
		{
			// Temporarily hide or visually mark as taken, then restock after a short delay
			ItemMesh->SetVisibility(false);
			bIsPurchased = true;

			GetWorld()->GetTimerManager().SetTimer(RestockTimerHandle, this, &ARealWorldStoreItem::ResetRestockState, 8.0f, false);
		}
		else
		{
			bIsPurchased = true;
			ItemMesh->SetVisibility(false);
		}
	}
}

void ARealWorldStoreItem::ResetRestockState()
{
	bIsPurchased = false;
	ItemMesh->SetVisibility(true);
}
