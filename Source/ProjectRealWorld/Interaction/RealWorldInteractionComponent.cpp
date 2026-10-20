#include "Interaction/RealWorldInteractionComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Character/RealWorldCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

URealWorldInteractionComponent::URealWorldInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f; // 20 Hz update
	MaxInteractionDistance = 280.0f;
	TraceRadius = 35.0f;
}

void URealWorldInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URealWorldInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateInteractionTrace();
}

void URealWorldInteractionComponent::UpdateInteractionTrace()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	APawn* OwnerPawn = Cast<APawn>(OwnerActor);
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return;

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector TraceStart = CameraLocation;
	const FVector TraceEnd = TraceStart + (CameraRotation.Vector() * MaxInteractionDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	QueryParams.bTraceComplex = false;

	FHitResult HitResult;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(TraceRadius);

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ECC_Visibility,
		SphereShape,
		QueryParams
	);

	AActor* CandidateActor = nullptr;
	FText CandidatePrompt = FText::GetEmpty();

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor->Implements<UInteractableInterface>())
		{
			// Strict Wall Occlusion Validation: Ensure player eyes have clear sightline to the target
			const FVector EyeLocation = OwnerPawn->GetPawnViewLocation();
			if (ValidateLineOfSight(HitActor, EyeLocation, HitResult.ImpactPoint))
			{
				ARealWorldCharacter* CharOwner = Cast<ARealWorldCharacter>(OwnerActor);
				if (IInteractableInterface::Execute_CanInteract(HitActor, CharOwner))
				{
					CandidateActor = HitActor;
					CandidatePrompt = IInteractableInterface::Execute_GetInteractionPrompt(HitActor, CharOwner);
				}
			}
		}
	}

	if (CandidateActor != FocusedActor.Get() || !CandidatePrompt.EqualTo(CachedPrompt))
	{
		FocusedActor = CandidateActor;
		CachedPrompt = CandidatePrompt;
		OnInteractableTargetChanged.Broadcast(CandidateActor, CachedPrompt);
	}
}

bool URealWorldInteractionComponent::ValidateLineOfSight(AActor* TargetActor, const FVector& EyeLocation, const FVector& TargetLocation) const
{
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(TargetActor);

	FHitResult WallHit;
	bool bObstructed = GetWorld()->LineTraceSingleByChannel(
		WallHit,
		EyeLocation,
		TargetLocation,
		ECC_WorldStatic,
		QueryParams
	);

	// If hit wall before reaching the interactable, line of sight is blocked
	return !bObstructed;
}

bool URealWorldInteractionComponent::PerformInteraction()
{
	if (!FocusedActor.IsValid())
	{
		return false;
	}

	AActor* Target = FocusedActor.Get();
	ARealWorldCharacter* CharOwner = Cast<ARealWorldCharacter>(GetOwner());

	if (Target->Implements<UInteractableInterface>())
	{
		if (IInteractableInterface::Execute_CanInteract(Target, CharOwner))
		{
			IInteractableInterface::Execute_Interact(Target, CharOwner);
			return true;
		}
	}

	return false;
}
