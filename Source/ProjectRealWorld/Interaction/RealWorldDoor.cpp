#include "Interaction/RealWorldDoor.h"
#include "Components/StaticMeshComponent.h"

ARealWorldDoor::ARealWorldDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameMesh"));
	SetRootComponent(FrameMesh);
	FrameMesh->SetCollisionProfileName(TEXT("BlockAll"));

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(FrameMesh);
	DoorMesh->SetCollisionProfileName(TEXT("BlockAll"));

	bIsOpen = false;
	bIsLocked = false;
	OpenAngle = 90.0f;
	SwingSpeed = 5.0f;
}

void ARealWorldDoor::BeginPlay()
{
	Super::BeginPlay();
	ClosedRotation = DoorMesh->GetRelativeRotation();
	OpenRotation = ClosedRotation + FRotator(0.0f, OpenAngle, 0.0f);
}

void ARealWorldDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FRotator TargetRot = bIsOpen ? OpenRotation : ClosedRotation;
	const FRotator CurrentRot = DoorMesh->GetRelativeRotation();

	if (!CurrentRot.Equals(TargetRot, 0.5f))
	{
		DoorMesh->SetRelativeRotation(FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, SwingSpeed));
	}
}

FText ARealWorldDoor::GetInteractionPrompt_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	if (bIsLocked)
	{
		return FText::FromString(TEXT("Door Locked"));
	}
	return bIsOpen ? FText::FromString(TEXT("Close Door")) : FText::FromString(TEXT("Open Door"));
}

bool ARealWorldDoor::CanInteract_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	return !bIsLocked;
}

void ARealWorldDoor::Interact_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	if (!bIsLocked)
	{
		bIsOpen = !bIsOpen;
	}
}
