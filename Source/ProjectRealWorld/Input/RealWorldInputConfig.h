#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RealWorldInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * Data asset containing references to all Enhanced Input Actions and Contexts for Project RealWorld.
 * Allows designer-driven input remapping without modifying gameplay classes.
 */
UCLASS(BlueprintType, Const)
class PROJECTREALWORLD_API URealWorldInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Input Mapping Contexts ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InputContexts")
	TObjectPtr<UInputMappingContext> IMC_OnFoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InputContexts")
	TObjectPtr<UInputMappingContext> IMC_Vehicle;

	// --- On-Foot Actions ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Movement")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Movement")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Movement")
	TObjectPtr<UInputAction> IA_LookMouse;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Movement")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Movement")
	TObjectPtr<UInputAction> IA_Sprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Movement")
	TObjectPtr<UInputAction> IA_Crouch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Movement")
	TObjectPtr<UInputAction> IA_Vault;

	// --- Interaction Actions ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Interaction")
	TObjectPtr<UInputAction> IA_Interact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Interaction")
	TObjectPtr<UInputAction> IA_EnterVehicle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Interaction")
	TObjectPtr<UInputAction> IA_ExitVehicle;

	// --- Combat Actions ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Combat")
	TObjectPtr<UInputAction> IA_Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Combat")
	TObjectPtr<UInputAction> IA_Aim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Combat")
	TObjectPtr<UInputAction> IA_Reload;
};
