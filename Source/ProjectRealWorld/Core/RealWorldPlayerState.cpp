#include "Core/RealWorldPlayerState.h"

ARealWorldPlayerState::ARealWorldPlayerState()
{
	Cash = 1500.0f;
	BankBalance = 8500.0f;
	WantedLevel = 0;
}

void ARealWorldPlayerState::AddCash(float Amount, const FText& Description, EEconomyTransactionType Type)
{
	if (Amount > 0.0f)
	{
		Cash += Amount;
		LogTransaction(Type, Amount, Description.IsEmpty() ? FText::FromString(TEXT("Cash Credit")) : Description, true);
		OnCashChanged.Broadcast(Cash);
	}
}

bool ARealWorldPlayerState::SpendCash(float Amount, const FText& Description, EEconomyTransactionType Type)
{
	if (Amount <= 0.0f)
	{
		return false;
	}

	if (Cash >= Amount)
	{
		Cash -= Amount;
		LogTransaction(Type, -Amount, Description.IsEmpty() ? FText::FromString(TEXT("Purchase Debit")) : Description, true);
		OnCashChanged.Broadcast(Cash);
		return true;
	}

	// Insufficient funds
	LogTransaction(Type, -Amount, Description.IsEmpty() ? FText::FromString(TEXT("Purchase Failed - Insufficient Cash")) : Description, false);
	OnTransactionFailed.Broadcast(Type, FText::FromString(TEXT("Insufficient cash funds.")));
	return false;
}

bool ARealWorldPlayerState::DepositToBank(float Amount)
{
	if (Amount > 0.0f && Cash >= Amount)
	{
		Cash -= Amount;
		BankBalance += Amount;
		LogTransaction(EEconomyTransactionType::BankDeposit, Amount, FText::FromString(TEXT("ATM Deposit")), true);
		OnCashChanged.Broadcast(Cash);
		OnBankBalanceChanged.Broadcast(BankBalance);
		return true;
	}

	OnTransactionFailed.Broadcast(EEconomyTransactionType::BankDeposit, FText::FromString(TEXT("Cannot deposit more cash than carried.")));
	return false;
}

bool ARealWorldPlayerState::WithdrawFromBank(float Amount)
{
	if (Amount > 0.0f && BankBalance >= Amount)
	{
		BankBalance -= Amount;
		Cash += Amount;
		LogTransaction(EEconomyTransactionType::BankWithdrawal, Amount, FText::FromString(TEXT("ATM Withdrawal")), true);
		OnCashChanged.Broadcast(Cash);
		OnBankBalanceChanged.Broadcast(BankBalance);
		return true;
	}

	OnTransactionFailed.Broadcast(EEconomyTransactionType::BankWithdrawal, FText::FromString(TEXT("Insufficient bank account balance.")));
	return false;
}

void ARealWorldPlayerState::SetWantedLevel(int32 NewLevel)
{
	int32 ClampedLevel = FMath::Clamp(NewLevel, 0, 5);
	if (WantedLevel != ClampedLevel)
	{
		WantedLevel = ClampedLevel;
		OnWantedLevelChanged.Broadcast(WantedLevel);
	}
}

void ARealWorldPlayerState::AddInventoryItem(FName ItemID)
{
	if (!ItemID.IsNone() && !OwnedItemIDs.Contains(ItemID))
	{
		OwnedItemIDs.Add(ItemID);
	}
}

bool ARealWorldPlayerState::HasInventoryItem(FName ItemID) const
{
	return OwnedItemIDs.Contains(ItemID);
}

void ARealWorldPlayerState::LogTransaction(EEconomyTransactionType Type, float Amount, const FText& Description, bool bSuccess)
{
	FEconomyTransaction Tx;
	Tx.TransactionID = FGuid::NewGuid();
	Tx.Type = Type;
	Tx.Amount = Amount;
	Tx.Description = Description;
	Tx.Timestamp = FDateTime::UtcNow();
	Tx.bWasSuccessful = bSuccess;

	TransactionHistory.Add(Tx);

	// Keep last 100 transactions in history
	if (TransactionHistory.Num() > 100)
	{
		TransactionHistory.RemoveAt(0);
	}
}
