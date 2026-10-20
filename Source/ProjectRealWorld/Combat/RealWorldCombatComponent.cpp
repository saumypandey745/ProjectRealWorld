#include "Combat/RealWorldCombatComponent.h"
#include "Character/RealWorldCharacter.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

URealWorldCombatComponent::URealWorldCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsInCover = false;
	bIsAiming = false;
}

void URealWorldCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URealWorldCombatComponent::EquipWeapon(TSubclassOf<ARealWorldWeaponBase> WeaponClass)
{
	if (!WeaponClass) return;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	ARealWorldCharacter* CharOwner = Cast<ARealWorldCharacter>(OwnerActor);
	if (!CharOwner) return;

	// Destroy previously held weapon
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;
	SpawnParams.Instigator = CharOwner;

	CurrentWeapon = GetWorld()->SpawnActor<ARealWorldWeaponBase>(WeaponClass, CharOwner->GetActorTransform(), SpawnParams);

	if (CurrentWeapon && CharOwner->GetMesh())
	{
		// Attach to weapon grip socket on right hand
		CurrentWeapon->AttachToComponent(
			CharOwner->GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("hand_rSocket")
		);

		OnWeaponEquipped.Broadcast(CurrentWeapon);
	}
}

bool URealWorldCombatComponent::FireCurrentWeapon()
{
	if (!CurrentWeapon) return false;

	ARealWorldCharacter* CharOwner = Cast<ARealWorldCharacter>(GetOwner());
	if (!CharOwner) return false;

	return CurrentWeapon->FireWeapon(CharOwner);
}

bool URealWorldCombatComponent::ReloadCurrentWeapon()
{
	if (!CurrentWeapon) return false;
	return CurrentWeapon->Reload();
}

void URealWorldCombatComponent::SetAiming(bool bNewAiming)
{
	bIsAiming = bNewAiming;
	if (ARealWorldCharacter* CharOwner = Cast<ARealWorldCharacter>(GetOwner()))
	{
		CharOwner->CurrentUpperBodyState = bNewAiming ? EUpperBodyState::Aiming : EUpperBodyState::Relaxed;
	}
}

bool URealWorldCombatComponent::TryEnterCover()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;

	const FVector Start = OwnerActor->GetActorLocation();
	const FVector End = Start + (OwnerActor->GetActorForwardVector() * 110.0f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

	if (bHit)
	{
		bIsInCover = true;
		return true;
	}

	return false;
}

void URealWorldCombatComponent::ExitCover()
{
	bIsInCover = false;
}
