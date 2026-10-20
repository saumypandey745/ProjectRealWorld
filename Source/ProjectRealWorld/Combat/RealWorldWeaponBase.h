#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealWorldWeaponBase.generated.h"

class USkeletalMeshComponent;
class ARealWorldCharacter;
class UParticleSystem;
class USoundBase;

UENUM(BlueprintType)
enum class EWeaponClass : uint8
{
	Melee UMETA(DisplayName = "Melee"),
	Pistol UMETA(DisplayName = "Pistol"),
	SMG UMETA(DisplayName = "Submachine Gun"),
	AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	Shotgun UMETA(DisplayName = "Shotgun"),
	SniperRifle UMETA(DisplayName = "Sniper Rifle")
};

UCLASS()
class PROJECTREALWORLD_API ARealWorldWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ARealWorldWeaponBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Mesh")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config")
	EWeaponClass WeaponClass = EWeaponClass::Pistol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float BaseDamage = 32.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float FireRateRPM = 400.0f; // Rounds per minute

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float EffectiveRange = 4500.0f; // 45 meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MagazineCapacity = 15;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmoInMag = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 TotalReserveAmmo = 60;

	/** Trigger weapon discharge */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Actions")
	virtual bool FireWeapon(ARealWorldCharacter* InstigatorPawn);

	/** Reload magazine from reserves */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Actions")
	virtual bool Reload();

protected:
	virtual void BeginPlay() override;

	float LastFireTime = 0.0f;
};
