#include "Subsystems/CrimeDetectionSubsystem.h"
#include "Subsystems/PoliceWantedSubsystem.h"
#include "Subsystems/WeatherSubsystem.h"
#include "AI/RealWorldNPCCharacter.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"

void UCrimeDetectionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ActiveEvidence.Empty();
	PendingCalls.Empty();
	WorldClock = 0.0f;
}

void UCrimeDetectionSubsystem::Tick(float DeltaTime)
{
	WorldClock += DeltaTime;

	// Fetch current weather for evidence wash and acoustic attenuation
	float RainIntensity = 0.0f;
	if (UWeatherSubsystem* Weather = GetWorld()->GetSubsystem<UWeatherSubsystem>())
	{
		RainIntensity = Weather->CurrentWeatherState.RainIntensity;
	}

	// 1. Process pending 911 civilian witness calls
	UPoliceWantedSubsystem* PoliceSubsystem = GetWorld()->GetSubsystem<UPoliceWantedSubsystem>();

	for (int32 i = PendingCalls.Num() - 1; i >= 0; --i)
	{
		FWitnessReportCall& Call = PendingCalls[i];

		// Check if witness was incapacitated before finishing report
		if (!Call.Witness.IsValid() || Call.Witness->CurrentState == ENPCState::INJURED || Call.Witness->CurrentState == ENPCState::DEAD)
		{
			PendingCalls.RemoveAt(i);
			continue;
		}

		Call.CallDurationRemaining -= DeltaTime;
		if (Call.CallDurationRemaining <= 0.0f)
		{
			// Call successfully reached dispatch
			if (PoliceSubsystem)
			{
				PoliceSubsystem->ReportCrime(
					Call.CrimeData.CrimeType,
					Call.CrimeData.CrimeLocation,
					Call.CrimeData.Perpetrator.Get(),
					false // Civilian witness report with delay
				);
			}

			// NPC resumes safe fleeing
			if (Call.Witness.IsValid())
			{
				Call.Witness->SetNPCState(ENPCState::FLEEING);
			}

			PendingCalls.RemoveAt(i);
		}
	}

	// 2. Physical Evidence Decay (accelerated by rain)
	const float DecayRate = 1.0f + (RainIntensity * 2.0f); // Rain washes away evidence 3x faster
	for (int32 i = ActiveEvidence.Num() - 1; i >= 0; --i)
	{
		ActiveEvidence[i].LifetimeRemaining -= (DecayRate * DeltaTime);
		if (ActiveEvidence[i].LifetimeRemaining <= 0.0f)
		{
			ActiveEvidence.RemoveAt(i);
		}
	}
}

TStatId UCrimeDetectionSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCrimeDetectionSubsystem, STATGROUP_Tickables);
}

void UCrimeDetectionSubsystem::RecordCrime(ECrimeType CrimeType, FVector Location, AActor* Perpetrator, float NoiseMagnitude)
{
	FCrimeEvent Crime;
	Crime.CrimeID = FGuid::NewGuid();
	Crime.CrimeType = CrimeType;
	Crime.CrimeLocation = Location;
	Crime.WorldTimestamp = WorldClock;
	Crime.Perpetrator = Perpetrator;
	Crime.bReportedToPolice = false;

	// Assign severity
	switch (CrimeType)
	{
	case ECrimeType::MinorTheft:
	case ECrimeType::RecklessDriving:
		Crime.Severity = 0.25f;
		break;
	case ECrimeType::Assault:
	case ECrimeType::GrandTheftAuto:
		Crime.Severity = 0.5f;
		break;
	case ECrimeType::DischargingFirearm:
		Crime.Severity = 0.85f;
		break;
	case ECrimeType::Homicide:
		Crime.Severity = 1.0f;
		break;
	}

	// Weather acoustic attenuation
	float RainIntensity = 0.0f;
	if (UWeatherSubsystem* Weather = GetWorld()->GetSubsystem<UWeatherSubsystem>())
	{
		RainIntensity = Weather->CurrentWeatherState.RainIntensity;
	}

	UPoliceWantedSubsystem* PoliceSubsystem = GetWorld()->GetSubsystem<UPoliceWantedSubsystem>();

	// Scan nearby NPCs for perception
	for (TActorIterator<ARealWorldNPCCharacter> It(GetWorld()); It; ++It)
	{
		ARealWorldNPCCharacter* NPC = *It;
		if (!NPC || NPC->CurrentState == ENPCState::DEAD || NPC->CurrentState == ENPCState::INJURED) continue;

		const FVector NPCLegs = NPC->GetActorLocation();
		const float Distance = FVector::Dist(NPCLegs, Location);

		// Police Officer: Direct authority witness
		if (NPC->Archetype == ENPCArchetype::Police)
		{
			if (Distance < 4000.0f && CheckLineOfSight(NPC->GetActorLocation() + FVector(0,0,70), Location + FVector(0,0,50), NPC))
			{
				if (PoliceSubsystem)
				{
					PoliceSubsystem->ReportCrime(CrimeType, Location, Perpetrator, true); // Direct visual
				}
				Crime.bReportedToPolice = true;
				Crime.DetectionConfidence = 1.0f;
				break;
			}
		}

		// Civilian Witness Detection
		bool bDetected = false;

		// 1. Visual Line of Sight Check (within 25m, forward cone 140 degrees)
		if (Distance < 2500.0f)
		{
			const FVector DirToCrime = (Location - NPCLegs).GetSafeNormal();
			const float FacingDot = FVector::DotProduct(NPC->GetActorForwardVector(), DirToCrime);

			if (FacingDot > 0.25f) // Within forward ~150 degree cone
			{
				if (CheckLineOfSight(NPC->GetActorLocation() + FVector(0,0,70), Location + FVector(0,0,50), NPC))
				{
					bDetected = true;
					Crime.DetectionConfidence = FMath::Max(Crime.DetectionConfidence, 0.9f);
				}
			}
		}

		// 2. Auditory Perception (e.g. gunfire or screeching tires)
		if (!bDetected)
		{
			const float AttenuatedNoise = CalculateHearingAttenuation(NoiseMagnitude, Distance, RainIntensity);
			if (AttenuatedNoise > 0.35f)
			{
				bDetected = true;
				Crime.DetectionConfidence = FMath::Max(Crime.DetectionConfidence, 0.5f);
			}
		}

		if (bDetected)
		{
			Crime.Witnesses.Add(NPC);
			NPC->ReactToDanger(Location);
			NPC->SetNPCState(ENPCState::CALLING_POLICE);

			FWitnessReportCall NewCall;
			NewCall.Witness = NPC;
			NewCall.CrimeData = Crime;
			NewCall.CallDurationRemaining = FMath::FRandRange(7.0f, 12.0f); // Believable civilian 911 call delay
			PendingCalls.Add(NewCall);

			OnWitnessAlerted.Broadcast(NPC, Crime);
		}
	}

	SpawnPhysicalEvidence(CrimeType, Location);
	OnCrimeCommitted.Broadcast(Crime);
}

void UCrimeDetectionSubsystem::SpawnPhysicalEvidence(ECrimeType CrimeType, FVector Location)
{
	FPhysicalEvidence Evidence;
	Evidence.EvidenceID = FGuid::NewGuid();
	Evidence.RelatedCrime = CrimeType;
	Evidence.Location = Location;
	Evidence.LifetimeRemaining = 180.0f;
	Evidence.bDiscoveredByPolice = false;

	ActiveEvidence.Add(Evidence);
}

bool UCrimeDetectionSubsystem::CheckLineOfSight(const FVector& ObserverEye, const FVector& TargetPoint, AActor* IgnoredObserver) const
{
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(IgnoredObserver);

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, ObserverEye, TargetPoint, ECC_Visibility, Params);
	return !bHit || FVector::Dist(Hit.ImpactPoint, TargetPoint) < 100.0f;
}

float UCrimeDetectionSubsystem::CalculateHearingAttenuation(float RawNoise, float Distance, float RainIntensity) const
{
	if (Distance <= 0.0f) return RawNoise;

	// Attenuation: Noise drops with inverse distance square, plus environmental rain dampening
	const float DistanceFactor = FMath::Clamp(1.0f - (Distance / 5000.0f), 0.0f, 1.0f);
	const float WeatherDampening = 1.0f - (RainIntensity * 0.4f); // Heavy rain dampens sound by up to 40%

	return RawNoise * DistanceFactor * WeatherDampening;
}
