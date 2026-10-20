#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RealWorldPlayerState.generated.h"

UENUM(BlueprintType)
enum class EEconomyTransactionType : uint8
{
	MissionReward UMETA(DisplayName = "Mission Reward"),
	StorePurchase UMETA(DisplayName = "Store Purchase"),
	VehiclePurchase UMETA(DisplayName = "Vehicle Purchase"),
	BankDeposit UMETA(DisplayName = "Bank Deposit"),
	BankWithdrawal UMETA(DisplayName = "Bank Withdrawal"),
	FineOrPenalty UMETA(DisplayName = "Fine or Penalty")
};

USTRUCT(BlueprintType)
struct FEconomyTransaction
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	FGuid TransactionID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	EEconomyTransactionType Type = EEconomyTransactionType::StorePurchase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	float Amount = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	FText Description;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	FDateTime Timestamp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	bool bWasSuccessful = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCashChanged, float, NewCash);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBankBalanceChanged, float, NewBankBalance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWantedLevelChanged, int32, NewWantedLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTransactionFailed, EEconomyTransactionType, Type, const FText&, Reason);

UCLASS()
class PROJECTREALWORLD_API ARealWorldPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ARealWorldPlayerState();

	// --- Authoritative Economy System ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	float Cash = 1500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	float BankBalance = 8500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	TArray<FEconomyTransaction> TransactionHistory;

	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AddCash(float Amount, const FText& Description = FText::GetEmpty(), EEconomyTransactionType Type = EEconomyTransactionType::MissionReward);

	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool SpendCash(float Amount, const FText& Description = FText::GetEmpty(), EEconomyTransactionType Type = EEconomyTransactionType::StorePurchase);

	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool DepositToBank(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool WithdrawFromBank(float Amount);

	UFUNCTION(BlueprintPure, Category = "Economy")
	bool CanAfford(float Amount) const { return Cash >= Amount; }

	// --- Police Wanted Level (0 - 5 Stars) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Law")
	int32 WantedLevel = 0;

	UFUNCTION(BlueprintCallable, Category = "Law")
	void SetWantedLevel(int32 NewLevel);

	// --- Inventory Handles ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FName> OwnedItemIDs;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddInventoryItem(FName ItemID);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasInventoryItem(FName ItemID) const;

	// --- Events for UI ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCashChanged OnCashChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBankBalanceChanged OnBankBalanceChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWantedLevelChanged OnWantedLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTransactionFailed OnTransactionFailed;

private:
	void LogTransaction(EEconomyTransactionType Type, float Amount, const FText& Description, bool bSuccess);
};
