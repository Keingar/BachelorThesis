// Fill out your copyright notice in the Description page of Project Settings.
#include "MyBuoyancyComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UMyBuoyancyComponent::UMyBuoyancyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	ParentActor = GetOwner();
	SpeedPerSecond = 0.f;
}

void UMyBuoyancyComponent::BeginPlay()
{
	Super::BeginPlay();

	PreviousRotation = ParentActor->GetActorRotation();

	FVector DefaultSetLocation = ParentActor->GetActorLocation();
	DefaultSetLocation.Z += artificalAdditionalHeight;
	ParentActor->SetActorLocation(DefaultSetLocation);

	FFTCalculator = InitializeWaterZoneReference();
	ActorNotMovingTranform = ParentActor->GetActorTransform();
	FindAverageLocationofPontoons();
}

void UMyBuoyancyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bWaterZoneValid || !MyStaticMeshComponent || BouyancyPoints.IsEmpty()) return;
	
	FVector BuoyancyLocation = ParentActor->GetActorLocation();

	if (SpeedPerSecond > 0) {
		BuoyancyLocation = GetNewMovingPosition(DeltaTime);
	}
	else {
		BuoyancyLocation = GetMultiBuoyancyLocation(DeltaTime);
	}

	BuoyancyLocation.Z += artificalAdditionalHeight;

	ParentActor->SetActorLocation(BuoyancyLocation);

	RotateOwner(DeltaTime);
	if (SpeedPerSecond > 0) {
		ActorNotMovingTranform = ParentActor->GetActorTransform();
	}
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

	if (DebugPoints) { DrawBuoyancyArrayDebugPoints(GetBuoyancyArray(BouyancyPoints)); }
#endif
}

FVector UMyBuoyancyComponent::GetBuoyancyLocation(FVector RelativeLocation)
{
	FVector BuoyancyLocation = FVector::ZeroVector;
	FVector WorldLocation = ActorNotMovingTranform.TransformPosition(RelativeLocation);
	if (FFTCalculator == nullptr) { return BuoyancyLocation; }

	FVector GridPointLocation = FVector(WorldLocation.X, WorldLocation.Y, -RelativeLocation.Z) / FFTCalculator->Scale * FFTCalculator->MultiplyScale;
	FVector Displacement = FFTCalculator->GetDisplacementAtPoint(GridPointLocation);

	BuoyancyLocation = GridPointLocation * FFTCalculator->Scale / FFTCalculator->MultiplyScale + Displacement / FFTCalculator->Scale / FFTCalculator->OverlapScale;

	return BuoyancyLocation;
}

FVector UMyBuoyancyComponent::GetMultiBuoyancyLocation(float DeltaTime)
{
	FVector BuoyancyLocation = GetBuoyancyLocation(AverageLocationofPontoons);

	if(MyStaticMeshComponent)
	{
		const FVector MeshWorldLoc = MyStaticMeshComponent->GetComponentLocation();

		const float BaseSmoothingSpeed = 0.1f;

		FVector Result = BuoyancyLocation;

		FVector DeltaXY = FVector(
			BuoyancyLocation.X - MeshWorldLoc.X,
			BuoyancyLocation.Y - MeshWorldLoc.Y,
			0.f
		);

		float Distance = DeltaXY.Size();
		float AdaptiveSpeed = BaseSmoothingSpeed * FMath::Clamp(Distance / 100.f, 1.f, 3.f);

		Result.X = FMath::FInterpTo(MeshWorldLoc.X, BuoyancyLocation.X, DeltaTime, AdaptiveSpeed);
		Result.Y = FMath::FInterpTo(MeshWorldLoc.Y, BuoyancyLocation.Y, DeltaTime, AdaptiveSpeed);
		Result.Z = BuoyancyLocation.Z;

		return Result;
	}

	return BuoyancyLocation;
}

FVector UMyBuoyancyComponent::GetNewMovingPosition(float DeltaTime)
{
	if (!ParentActor || !MyStaticMeshComponent) return FVector::ZeroVector;

	FVector ResultLocation = ParentActor->GetActorLocation();

	FVector MeshForward = MyStaticMeshComponent->GetComponentTransform().TransformVectorNoScale(FVector::ForwardVector);

	const FRotator MeshYawOffset(0.f, 90.f, 0.f); // I use this only for ship and ship is not correctly placed so I do manual turn here
	FVector ForwardWorld = MeshYawOffset.RotateVector(MyStaticMeshComponent->GetComponentTransform().TransformVectorNoScale(FVector::ForwardVector));

	ForwardWorld = ForwardWorld.GetSafeNormal();

	ResultLocation += ForwardWorld * SpeedPerSecond * DeltaTime;
	ResultLocation.Z = GetBuoyancyLocation(AverageLocationofPontoons).Z; // from waves only use height

	return ResultLocation;
}

FOceanFFTCalculator* UMyBuoyancyComponent::InitializeWaterZoneReference()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOceanWaterZone::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		for (AActor* FoundActor : FoundActors)
		{
			OceanWaterZone = Cast<AOceanWaterZone>(FoundActor);
			if (OceanWaterZone)
			{
				return &OceanWaterZone->FFTCalculator;
			}
		}
	}
	bWaterZoneValid = false;
	UE_LOG(LogTemp, Warning, TEXT("WaterZone isn't valid"));
	return nullptr;
}

void UMyBuoyancyComponent::FindAverageLocationofPontoons()
{
	FVector AverageLocation = FVector::ZeroVector;
	for (FVector& Location : BouyancyPoints)
	{
		AverageLocation = AverageLocation + Location;
	}
	AverageLocation = AverageLocation / BouyancyPoints.Num();
	AverageLocationofPontoons = AverageLocation;
}

TArray<FVector> UMyBuoyancyComponent::GetBuoyancyArray(TArray<FVector> Points)
{
	TArray<FVector> PointArray = {};
	for (FVector& Point : Points)
	{
		PointArray.Add(GetBuoyancyLocation(Point));
	}
	return PointArray;
}

void UMyBuoyancyComponent::RotateOwner(float DeltaTime)
{
	if (!MyStaticMeshComponent || !ParentActor) return;

	FVector TotalTorque = FVector::ZeroVector;
	int32 ContributingPoints = 0;
	FVector ActorLocation = ParentActor->GetActorLocation();
	ActorLocation.Z -= artificalAdditionalHeight;
	for (const FVector& LocalPoint : BouyancyPoints)
	{
		const FVector WorldPoint = MyStaticMeshComponent->GetComponentTransform().TransformPosition(LocalPoint);
		const FVector WavePoint = GetBuoyancyLocation(LocalPoint);
		const float SubmersionDepth = WavePoint.Z - WorldPoint.Z;

		if (FMath::Abs(SubmersionDepth) < 10.f) continue;

		const FVector r = WorldPoint - ActorLocation;
		FVector Force = FVector::UpVector * SubmersionDepth;
		if (SubmersionDepth < 0.f)
			Force *= 0.3f;

		const FVector Torque = FVector::CrossProduct(r, Force);
		TotalTorque += Torque;
		ContributingPoints++;

		if (DebugPoints)
		{
			FColor Color = (SubmersionDepth > 0.f) ? FColor::Red : FColor::Blue;
			DrawDebugLine(GetWorld(), WorldPoint, WorldPoint + Force * 3.f, Color, false, -1.f, 0, 30.f);
		}
	}

	if (ContributingPoints == 0) return;

	TotalTorque /= ContributingPoints;

	// --- nonlinear response section ---
	float TorqueMagnitude = TotalTorque.Size();
	const float SmallWaveThreshold = 70000.f;
	const float BigWaveThreshold = 120000.f;

	float t = FMath::Clamp((TorqueMagnitude - SmallWaveThreshold) / (BigWaveThreshold - SmallWaveThreshold), 0.f, 1.f);
	t = FMath::Pow(t, 1.5f);

	float StrengthFactor = FMath::Pow(t, 2.0f);
	if (TorqueMagnitude < SmallWaveThreshold)
		TotalTorque *= 0.05f;
	else
		TotalTorque *= StrengthFactor;

	const float MinResponse = 0.35f;
	const float MaxResponse = 0.9f;
	float TorqueResponseSpeed = FMath::Lerp(MinResponse, MaxResponse, t);

	// Optional boost when torque changes quickly
	float TorqueChange = (TotalTorque - PreviousTorque).Size();
	float Boost = FMath::Clamp(TorqueChange / 50000.f, 0.f, 1.f);
	TorqueResponseSpeed += Boost * 0.4f;

	// Smoothly interpolate response speed
	const float ResponseInterpSpeed = 2.0f;
	SmoothedResponseSpeed = FMath::FInterpTo(
		SmoothedResponseSpeed,
		TorqueResponseSpeed,
		DeltaTime,
		ResponseInterpSpeed
	);

	const FVector LocalTorque = ParentActor->GetActorTransform().InverseTransformVectorNoScale(TotalTorque);
	float TargetPitch = -LocalTorque.Y * PitchStrength;
	float TargetRoll = -LocalTorque.X * RollStrength;

	// --- Apply damping before clamping ---
	FRotator RotationDelta = (ParentActor->GetActorRotation() - PreviousRotation).GetNormalized();
	float AdaptiveDamping = FMath::Lerp(0.25f, 0.05f, t); // stronger damping in calm water
	TargetPitch -= RotationDelta.Pitch * AdaptiveDamping;
	TargetRoll -= RotationDelta.Roll * AdaptiveDamping;
	PreviousRotation = ParentActor->GetActorRotation();

	// --- Natural leveling (only once) ---
	const float LevelingStrength = 0.1f;
	TargetPitch -= ParentActor->GetActorRotation().Pitch * LevelingStrength;
	TargetRoll -= ParentActor->GetActorRotation().Roll * LevelingStrength;

	// Clamp after damping + leveling
	TargetPitch = FMath::Clamp(TargetPitch, -32.f, 32.f);
	TargetRoll = FMath::Clamp(TargetRoll, -40.f, 40.f);

	const FRotator CurrentRotation = ParentActor->GetActorRotation();
	const FRotator InstantTarget(TargetPitch, CurrentRotation.Yaw, TargetRoll);

	SmoothedTargetRotation = FMath::RInterpTo(
		SmoothedTargetRotation,
		InstantTarget,
		DeltaTime,
		SmoothedResponseSpeed * 3.0f
	);

	FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		SmoothedTargetRotation,
		DeltaTime,
		SmoothedResponseSpeed * 0.5f
	);
	NewRotation.Yaw = CurrentRotation.Yaw;

	ParentActor->SetActorRotation(NewRotation);
}

void UMyBuoyancyComponent::DrawBuoyancyArrayDebugPoints(const TArray<FVector>& BuoyancyArray)
{
	for (const FVector& Point : BuoyancyArray)
	{
		DrawDebugPoint(GetWorld(), Point, 50.f, FColor(255.f, 0.f, 0.f, 255.f), false, 0.f, 0);
	}
}