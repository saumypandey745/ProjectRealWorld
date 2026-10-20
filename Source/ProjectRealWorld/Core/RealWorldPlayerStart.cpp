#include "Core/RealWorldPlayerStart.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

ARealWorldPlayerStart::ARealWorldPlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DistrictTag = TEXT("Downtown");
	InitialCameraYaw = 0.0f;
}

bool ARealWorldPlayerStart::ValidateSpawnClearance() const
{
	const FVector Location = GetActorLocation();
	const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(34.0f, 90.0f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	return !GetWorld()->OverlapBlockingTestByChannel(
		Location,
		FQuat::Identity,
		ECC_Pawn,
		CapsuleShape,
		QueryParams
	);
}
