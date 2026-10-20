#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/RealWorldWeaponBase.h"
#include "RealWorldCombatComponent.generated.h"

class ARealWorldCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEquipped, ARealWorldWeaponBase*, NewWeapon);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREALWORLD_API URealWorldCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URealWorldCombatComponent();

	virtual void BeginPlay() override;

	/** Currently equipped active weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	TObjectPtr<ARealWorldWeaponBase> CurrentWeapon;

	/** In cover flag */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	bool bIsInCover = false;

	/** Is character currently aiming down sights */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	bool bIsAiming = false;

	/** Equip weapon class */
	UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
	void EquipWeapon(TSubclassOf<ARealWorldWeaponBase> WeaponClass);

	/** Fire current weapon */
	UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
	bool FireCurrentWeapon();

	/** Reload current weapon */
	UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
	bool ReloadCurrentWeapon();

	/** Set aiming state */
	UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
	void SetAiming(bool bNewAiming);

	/** Toggle cover mode against wall */
	UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
	bool TryEnterCover();

	UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
	void ExitCover();

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnWeaponEquipped OnWeaponEquipped;
};
