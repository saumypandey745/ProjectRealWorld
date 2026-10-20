#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "RealWorldStoreItem.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EStoreItemCategory : uint8
{
	Beverage UMETA(DisplayName = "Beverage"),
	Snack UMETA(DisplayName = "Snack Food"),
	FirstAid UMETA(DisplayName = "First Aid"),
	Electronics UMETA(DisplayName = "Electronics"),
	Magazine UMETA(DisplayName = "Magazine")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStoreItemPurchased, FName, ItemID, float, PricePaid, AActor*, Purchaser);

UCLASS()
class PROJECTREALWORLD_API ARealWorldStoreItem : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ARealWorldStoreItem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Store|Components")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Store|Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Config")
	FName ItemID = FName(TEXT("ITEM_EnergyDrink"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Config")
	FText ItemName = FText::FromString(TEXT("Nitro Energy Drink"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Config")
	FText ItemDescription = FText::FromString(TEXT("Restores 35 stamina and boosts sprint recovery."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Config")
	EStoreItemCategory Category = EStoreItemCategory::Beverage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Config")
	float Price = 4.50f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Stock")
	bool bInfiniteStock = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Store|Stock")
	bool bIsPurchased = false;

	UPROPERTY(BlueprintAssignable, Category = "Store|Events")
	FOnStoreItemPurchased OnStoreItemPurchased;

	// --- IInteractableInterface ---
	virtual FText GetInteractionPrompt_Implementation(ARealWorldCharacter* InstigatorCharacter) override;
	virtual bool CanInteract_Implementation(ARealWorldCharacter* InstigatorCharacter) override;
	virtual void Interact_Implementation(ARealWorldCharacter* InstigatorCharacter) override;

protected:
	virtual void BeginPlay() override;
	void ResetRestockState();

private:
	FTimerHandle RestockTimerHandle;
};
