#include "AI/RealWorldNPCCharacter.h"
#include "AI/RealWorldNPCAIController.h"
#include "Subsystems/TimeOfDaySubsystem.h"
#include "Subsystems/WeatherSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ARealWorldNPCCharacter::ARealWorldNPCCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;

	AIControllerClass = ARealWorldNPCAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 90.0f);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 180.0f;

	// Modular mesh components (MetaHuman lead pose pattern)
	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(GetMesh());
	HeadMesh->SetLeaderPoseComponent(GetMesh());

	TorsoMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TorsoMesh"));
	TorsoMesh->SetupAttachment(GetMesh());
	TorsoMesh->SetLeaderPoseComponent(GetMesh());

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh"));
	LegsMesh->SetupAttachment(GetMesh());
	LegsMesh->SetLeaderPoseComponent(GetMesh());

	FeetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FeetMesh"));
	FeetMesh->SetupAttachment(GetMesh());
	FeetMesh->SetLeaderPoseComponent(GetMesh());

	HairMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairMesh->SetupAttachment(HeadMesh);
	HairMesh->SetLeaderPoseComponent(HeadMesh);

	AccessoriesMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AccessoriesMesh"));
	AccessoriesMesh->SetupAttachment(GetMesh());
	AccessoriesMesh->SetLeaderPoseComponent(GetMesh());

	Archetype = ENPCArchetype::CivilianMale;
	CurrentEmotion = EFacialEmotionState::Neutral;
	StoreSubstate = EStoreInteractionSubstate::None;
	NPCName = FText::FromString(TEXT("Citizen"));
	Occupation = FText::FromString(TEXT("Civilian"));
	StoreActionTimer = 0.0f;
}

void ARealWorldNPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (DailySchedule.Num() == 0)
	{
		DailySchedule.Add({ 7.0f, ENPCState::GOING_HOME, FVector::ZeroVector, TEXT("Home") });
		DailySchedule.Add({ 8.5f, ENPCState::WALKING, FVector::ZeroVector, TEXT("Transit") });
		DailySchedule.Add({ 9.0f, ENPCState::WORKING, FVector::ZeroVector, TEXT("Office") });
		DailySchedule.Add({ 13.0f, ENPCState::EATING, FVector::ZeroVector, TEXT("Diner") });
		DailySchedule.Add({ 14.0f, ENPCState::WORKING, FVector::ZeroVector, TEXT("Office") });
		DailySchedule.Add({ 17.5f, ENPCState::SHOPPING, FVector::ZeroVector, TEXT("Market") });
		DailySchedule.Add({ 19.5f, ENPCState::SOCIALIZING, FVector::ZeroVector, TEXT("Park") });
		DailySchedule.Add({ 22.0f, ENPCState::GOING_HOME, FVector::ZeroVector, TEXT("Home") });
		DailySchedule.Add({ 23.5f, ENPCState::IDLE, FVector::ZeroVector, TEXT("Sleeping") });
	}

	SetArchetype(Archetype);
}

void ARealWorldNPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSimulationLOD();

	// Store Shopping State Machine execution
	if (CurrentState == ENPCState::SHOPPING)
	{
		UpdateStoreShoppingBehavior(DeltaTime);
	}

	// Schedule check every 5s
	ScheduleCheckTimer += DeltaTime;
	if (ScheduleCheckTimer >= 5.0f)
	{
		ScheduleCheckTimer = 0.0f;
		if (UWorld* World = GetWorld())
		{
			if (UTimeOfDaySubsystem* TimeSub = World->GetSubsystem<UTimeOfDaySubsystem>())
			{
				EvaluateDailySchedule(TimeSub->CurrentTimeHours);
			}

			if (UWeatherSubsystem* WeatherSub = World->GetSubsystem<UWeatherSubsystem>())
			{
				ReactToWeather(WeatherSub->CurrentProperties.RainIntensity);
			}
		}
	}
}

void ARealWorldNPCCharacter::SetArchetype(ENPCArchetype NewArchetype)
{
	Archetype = NewArchetype;

	switch (Archetype)
	{
	case ENPCArchetype::CivilianMale:
		Occupation = FText::FromString(TEXT("Citizen"));
		GetCharacterMovement()->MaxWalkSpeed = 180.0f;
		break;
	case ENPCArchetype::CivilianFemale:
		Occupation = FText::FromString(TEXT("Citizen"));
		GetCharacterMovement()->MaxWalkSpeed = 175.0f;
		break;
	case ENPCArchetype::Business:
		Occupation = FText::FromString(TEXT("Corporate Executive"));
		GetCharacterMovement()->MaxWalkSpeed = 200.0f; // Purposeful brisk stride
		break;
	case ENPCArchetype::Casual:
		Occupation = FText::FromString(TEXT("Local"));
		GetCharacterMovement()->MaxWalkSpeed = 160.0f;
		break;
	case ENPCArchetype::ShopWorker:
		Occupation = FText::FromString(TEXT("Store Clerk"));
		GetCharacterMovement()->MaxWalkSpeed = 150.0f;
		break;
	case ENPCArchetype::Security:
		Occupation = FText::FromString(TEXT("Security Officer"));
		GetCharacterMovement()->MaxWalkSpeed = 190.0f;
		break;
	case ENPCArchetype::MunicipalWorker:
		Occupation = FText::FromString(TEXT("City Maintenance"));
		GetCharacterMovement()->MaxWalkSpeed = 170.0f;
		break;
	case ENPCArchetype::Police:
		Occupation = FText::FromString(TEXT("Police Officer"));
		GetCharacterMovement()->MaxWalkSpeed = 220.0f;
		break;
	}
}

void ARealWorldNPCCharacter::SetFacialEmotion(EFacialEmotionState NewEmotion)
{
	CurrentEmotion = NewEmotion;
}

void ARealWorldNPCCharacter::UpdateSimulationLOD()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

	if (DistanceToPlayer < 2500.0f)
	{
		CurrentSimulationLOD = ENPCSimulationLOD::FullTick;
		PrimaryActorTick.TickInterval = 0.0f;
		GetMesh()->bPauseAnims = false;
	}
	else if (DistanceToPlayer < 7500.0f)
	{
		CurrentSimulationLOD = ENPCSimulationLOD::ReducedTick;
		PrimaryActorTick.TickInterval = 0.08f;
		GetMesh()->bPauseAnims = false;
	}
	else if (DistanceToPlayer < 20000.0f)
	{
		CurrentSimulationLOD = ENPCSimulationLOD::FarBudgeted;
		PrimaryActorTick.TickInterval = 0.5f;
		GetMesh()->bPauseAnims = true;
	}
	else
	{
		CurrentSimulationLOD = ENPCSimulationLOD::Dormant;
		PrimaryActorTick.TickInterval = 2.0f;
		GetMesh()->bPauseAnims = true;
	}
}

void ARealWorldNPCCharacter::EvaluateDailySchedule(float CurrentHour)
{
	if (CurrentState == ENPCState::FLEEING || CurrentState == ENPCState::FIGHTING || CurrentState == ENPCState::DEAD)
	{
		return;
	}

	ENPCState TargetScheduleState = CurrentState;

	for (int32 i = DailySchedule.Num() - 1; i >= 0; --i)
	{
		if (CurrentHour >= DailySchedule[i].StartHour)
		{
			TargetScheduleState = DailySchedule[i].AssignedState;
			break;
		}
	}

	if (TargetScheduleState != CurrentState)
	{
		SetNPCState(TargetScheduleState);
	}
}

void ARealWorldNPCCharacter::SetNPCState(ENPCState NewState)
{
	CurrentState = NewState;

	if (ARealWorldNPCAIController* AIC = Cast<ARealWorldNPCAIController>(GetController()))
	{
		AIC->OnNPCStateChanged(NewState);
	}
}

void ARealWorldNPCCharacter::ReactToDanger(FVector DangerSource)
{
	if (CurrentState == ENPCState::DEAD) return;

	SetNPCState(ENPCState::FLEEING);
	SetFacialEmotion(EFacialEmotionState::Fear);
	GetCharacterMovement()->MaxWalkSpeed = 520.0f; // Panic sprint away from threat
}

void ARealWorldNPCCharacter::ReactToWeather(float RainIntensity)
{
	if (CurrentState == ENPCState::FLEEING || CurrentState == ENPCState::DEAD) return;

	if (RainIntensity > 0.45f)
	{
		// Heavy precipitation: Hurry along the sidewalk or seek shelter under building awnings
		GetCharacterMovement()->MaxWalkSpeed = 260.0f;
	}
	else
	{
		SetArchetype(Archetype); // Restore normal gait
	}
}

void ARealWorldNPCCharacter::ReactToSprintProximity(const FVector& SprinterLocation)
{
	if (CurrentState == ENPCState::FLEEING || CurrentState == ENPCState::DEAD) return;

	// Turn body slightly away from approaching sprinter
	FVector AwayDir = (GetActorLocation() - SprinterLocation).GetSafeNormal2D();
	SetFacialEmotion(EFacialEmotionState::Surprise);

	// Quick sidestep
	AddMovementInput(AwayDir, 0.4f);
}

void ARealWorldNPCCharacter::ReactToCollision(AActor* CollidingActor, const FVector& HitNormal)
{
	if (CurrentState == ENPCState::DEAD) return;

	SetFacialEmotion(EFacialEmotionState::Angry);

	// Push back and stumble momentarily
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->AddImpulse(HitNormal * 300.0f, true);
	}
}

void ARealWorldNPCCharacter::ReactToWeaponDischarge(const FVector& GunshotLocation)
{
	if (CurrentState == ENPCState::DEAD) return;

	SetNPCState(ENPCState::FLEEING);
	SetFacialEmotion(EFacialEmotionState::Fear);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = 560.0f; // Maximum sprint speed
	}

	FVector FleeDir = (GetActorLocation() - GunshotLocation).GetSafeNormal2D();
	FVector FleeTarget = GetActorLocation() + (FleeDir * 2500.0f);
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), FleeTarget);
}

void ARealWorldNPCCharacter::ReactToVehicleAccident(const FVector& CrashLocation)
{
	if (CurrentState == ENPCState::DEAD) return;

	// Stop in place, face collision scene with startled expression
	SetNPCState(ENPCState::IDLE);
	SetFacialEmotion(EFacialEmotionState::Surprise);

	FVector LookDir = (CrashLocation - GetActorLocation()).GetSafeNormal2D();
	FRotator TargetRot = LookDir.Rotation();
	SetActorRotation(FRotator(0.0f, TargetRot.Yaw, 0.0f));
}

void ARealWorldNPCCharacter::StartStoreVisitRoutine(const FVector& StoreEntrance, const FVector& ShelfA, const FVector& Counter)
{
	TargetStoreEntrance = StoreEntrance;
	TargetShelf = ShelfA;
	TargetCounter = Counter;

	SetNPCState(ENPCState::SHOPPING);
	StoreSubstate = EStoreInteractionSubstate::ApproachStore;
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), TargetStoreEntrance);
}

void ARealWorldNPCCharacter::UpdateStoreShoppingBehavior(float DeltaTime)
{
	const float DistToDest = FVector::Dist(GetActorLocation(), GetVelocity().IsNearlyZero() ? GetActorLocation() : TargetStoreEntrance);

	switch (StoreSubstate)
	{
	case EStoreInteractionSubstate::ApproachStore:
		if (FVector::Dist(GetActorLocation(), TargetStoreEntrance) < 180.0f)
		{
			StoreSubstate = EStoreInteractionSubstate::EnterStore;
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), TargetShelf);
		}
		break;

	case EStoreInteractionSubstate::EnterStore:
		if (FVector::Dist(GetActorLocation(), TargetShelf) < 150.0f)
		{
			StoreSubstate = EStoreInteractionSubstate::BrowseAisles;
			StoreActionTimer = 5.0f; // Browse for 5 seconds
		}
		break;

	case EStoreInteractionSubstate::BrowseAisles:
		StoreActionTimer -= DeltaTime;
		if (StoreActionTimer <= 0.0f)
		{
			StoreSubstate = EStoreInteractionSubstate::ApproachCounter;
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), TargetCounter);
		}
		break;

	case EStoreInteractionSubstate::ApproachCounter:
		if (FVector::Dist(GetActorLocation(), TargetCounter) < 150.0f)
		{
			StoreSubstate = EStoreInteractionSubstate::Checkout;
			StoreActionTimer = 3.5f; // Pay at register for 3.5 seconds
		}
		break;

	case EStoreInteractionSubstate::Checkout:
		StoreActionTimer -= DeltaTime;
		if (StoreActionTimer <= 0.0f)
		{
			StoreSubstate = EStoreInteractionSubstate::ExitStore;
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), TargetStoreEntrance);
		}
		break;

	case EStoreInteractionSubstate::ExitStore:
		if (FVector::Dist(GetActorLocation(), TargetStoreEntrance) < 180.0f)
		{
			StoreSubstate = EStoreInteractionSubstate::None;
			SetNPCState(ENPCState::WALKING); // Resume street sidewalk schedule
		}
		break;

	default:
		break;
	}
}

FText ARealWorldNPCCharacter::GetInteractionPrompt_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	return FText::Format(FText::FromString(TEXT("Talk to {0} ({1})")), NPCName, Occupation);
}

bool ARealWorldNPCCharacter::CanInteract_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	return CurrentState != ENPCState::DEAD && CurrentState != ENPCState::FLEEING;
}

void ARealWorldNPCCharacter::Interact_Implementation(ARealWorldCharacter* InstigatorCharacter)
{
	if (CurrentState != ENPCState::SOCIALIZING)
	{
		SetNPCState(ENPCState::SOCIALIZING);
		SetFacialEmotion(EFacialEmotionState::Happy);
	}
}
