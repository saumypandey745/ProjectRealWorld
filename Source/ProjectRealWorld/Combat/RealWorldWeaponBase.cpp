#include "Combat/RealWorldWeaponBase.h"
#include "Character/RealWorldCharacter.h"
#include "Subsystems/PoliceWantedSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "CollisionQueryParams.h"

ARealWorldWeaponBase::ARealWorldWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	BaseDamage = 32.0f;
	FireRateRPM = 450.0f;
	EffectiveRange = 5000.0f;
	MagazineCapacity = 15;
	CurrentAmmoInMag = 15;
	TotalReserveAmmo = 60;
}

void ARealWorldWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmoInMag = MagazineCapacity;
}

bool ARealWorldWeaponBase::FireWeapon(ARealWorldCharacter* InstigatorPawn)
{
	if (!InstigatorPawn || CurrentAmmoInMag <= 0) return false;

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float MinFireInterval = 60.0f / FireRateRPM;

	if ((CurrentTime - LastFireTime) < MinFireInterval)
	{
		return false;
	}

	LastFireTime = CurrentTime;
	CurrentAmmoInMag--;

	// Acoustic perception stimulus for nearby NPCs and police patrols
	MakeNoise(1.0f, InstigatorPawn, GetActorLocation());

	// Line trace for hitscan ballistics
	FVector ViewLoc;
	FRotator ViewRot;
	InstigatorPawn->GetController()->GetPlayerViewPoint(ViewLoc, ViewRot);

	const FVector TraceStart = ViewLoc;
	const FVector TraceEnd = TraceStart + (ViewRot.Vector() * EffectiveRange);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(InstigatorPawn);

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	if (bHit && HitResult.GetActor())
	{
		UGameplayStatics::ApplyPointDamage(
			HitResult.GetActor(),
			BaseDamage,
			ViewRot.Vector(),
			HitResult,
			InstigatorPawn->GetController(),
			this,
			UDamageType::StaticClass()
		);
	}

	// Notify Police Wanted Subsystem if discharging in public city zones
	if (UPoliceWantedSubsystem* WantedSub = GetWorld()->GetSubsystem<UPoliceWantedSubsystem>())
	{
		WantedSub->ReportCrime(ECrimeType::DischargingFirearm, GetActorLocation(), InstigatorPawn, false);
	}

	return true;
}

bool ARealWorldWeaponBase::Reload()
{
	if (CurrentAmmoInMag >= MagazineCapacity || TotalReserveAmmo <= 0)
	{
		return false;
	}

	const int32 AmmoNeeded = MagazineCapacity - CurrentAmmoInMag;
	const int32 AmmoToTake = FMath::Min(AmmoNeeded, TotalReserveAmmo);

	CurrentAmmoInMag += AmmoToTake;
	TotalReserveAmmo -= AmmoToTake;
	return true;
}
