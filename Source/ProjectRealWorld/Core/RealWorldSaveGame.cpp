#include "Core/RealWorldSaveGame.h"

URealWorldSaveGame::URealWorldSaveGame()
{
	SaveVersion = CURRENT_SAVE_VERSION;
	SaveSlotName = TEXT("RealWorld_Autosave");
	Timestamp = FDateTime::UtcNow();
	ResetToSafeDefaults();
}

void URealWorldSaveGame::ResetToSafeDefaults()
{
	PlayerTransform = FTransform(FRotator(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 100.0f), FVector(1.0f, 1.0f, 1.0f));
	PlayerHealth = 100.0f;
	PlayerStamina = 100.0f;
	Cash = 1500.0f;
	BankBalance = 8500.0f;
	WantedLevel = 0;
	TimeOfDayHours = 8.5f; // Morning default 8:30 AM
	CurrentWeatherType = 0; // Clear
	InventoryItemIDs.Empty();
	OwnedVehicles.Empty();
	MissionStates.Empty();
}

bool URealWorldSaveGame::ValidateAndMigrateSaveData()
{
	// Check for version migration
	if (SaveVersion < 2)
	{
		// Upgrade from Version 1:
		// Assign default bank balance and stamina if missing
		if (BankBalance <= 0.0f)
		{
			BankBalance = 8500.0f;
		}
		if (PlayerStamina <= 0.0f)
		{
			PlayerStamina = 100.0f;
		}
		SaveVersion = CURRENT_SAVE_VERSION;
	}

	// Corruption safety checks
	bool bDataValid = true;

	// 1. Sanitize health and stamina
	if (FMath::IsNaN(PlayerHealth) || PlayerHealth <= 0.0f)
	{
		PlayerHealth = 100.0f;
		bDataValid = false;
	}
	else
	{
		PlayerHealth = FMath::Clamp(PlayerHealth, 1.0f, 1000.0f);
	}

	if (FMath::IsNaN(PlayerStamina) || PlayerStamina < 0.0f)
	{
		PlayerStamina = 100.0f;
		bDataValid = false;
	}
	else
	{
		PlayerStamina = FMath::Clamp(PlayerStamina, 0.0f, 1000.0f);
	}

	// 2. Sanitize economy
	if (FMath::IsNaN(Cash) || Cash < 0.0f)
	{
		Cash = 1500.0f;
		bDataValid = false;
	}

	if (FMath::IsNaN(BankBalance) || BankBalance < 0.0f)
	{
		BankBalance = 8500.0f;
		bDataValid = false;
	}

	// 3. Sanitize wanted level
	WantedLevel = FMath::Clamp(WantedLevel, 0, 5);

	// 4. Sanitize world time
	if (FMath::IsNaN(TimeOfDayHours) || TimeOfDayHours < 0.0f || TimeOfDayHours >= 24.0f)
	{
		TimeOfDayHours = 9.0f;
		bDataValid = false;
	}

	// 5. Sanitize transform
	FVector Pos = PlayerTransform.GetLocation();
	if (FMath::IsNaN(Pos.X) || FMath::IsNaN(Pos.Y) || FMath::IsNaN(Pos.Z) || Pos.ContainsNaN())
	{
		PlayerTransform = FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 100.0f), FVector::OneVector);
		bDataValid = false;
	}

	return bDataValid;
}
