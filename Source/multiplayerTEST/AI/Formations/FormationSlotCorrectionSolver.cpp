#include "FormationSlotCorrectionSolver.h"

#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "multiplayerTEST/Characters/Enemies/EnemyCore.h"

namespace
{
	FVector Flatten(const FVector& Value)
	{
		return FVector(Value.X, Value.Y, 0.f);
	}

	float GetRoleMobility(EFormationMemberRole Role, bool bIsLeader, bool bRoleAware)
	{
		if (bIsLeader || Role == EFormationMemberRole::Leader)
		{
			return 0.18f;
		}

		if (!bRoleAware)
		{
			return 1.f;
		}

		switch (Role)
		{
		case EFormationMemberRole::Defender:
			return 0.65f;
		case EFormationMemberRole::Aggressive:
			return 0.75f;
		case EFormationMemberRole::Support:
			return 1.15f;
		case EFormationMemberRole::Backline:
			return 1.25f;
		case EFormationMemberRole::Flanker:
			return 1.05f;
		default:
			return 1.f;
		}
	}

	float GetRoleShapePreservation(EFormationMemberRole Role, bool bIsLeader, bool bRoleAware)
	{
		if (bIsLeader || Role == EFormationMemberRole::Leader)
		{
			return 1.f;
		}

		if (!bRoleAware)
		{
			return 0.7f;
		}

		switch (Role)
		{
		case EFormationMemberRole::Defender:
			return 0.9f;
		case EFormationMemberRole::Aggressive:
			return 0.85f;
		case EFormationMemberRole::Support:
			return 0.55f;
		case EFormationMemberRole::Backline:
			return 0.45f;
		case EFormationMemberRole::Flanker:
			return 0.6f;
		default:
			return 0.7f;
		}
	}

	float GetRoleMaxOffsetScale(EFormationMemberRole Role, bool bIsLeader, bool bRoleAware)
	{
		if (bIsLeader || Role == EFormationMemberRole::Leader)
		{
			return 0.25f;
		}

		if (!bRoleAware)
		{
			return 1.f;
		}

		switch (Role)
		{
		case EFormationMemberRole::Defender:
			return 0.8f;
		case EFormationMemberRole::Aggressive:
			return 0.85f;
		case EFormationMemberRole::Support:
			return 1.1f;
		case EFormationMemberRole::Backline:
			return 1.2f;
		case EFormationMemberRole::Flanker:
			return 1.f;
		default:
			return 1.f;
		}
	}

	float GetMemberCollisionRadius(AEnemyCore* Member, float FallbackRadius, float RadiusScale)
	{
		float Radius = FallbackRadius;
		if (IsValid(Member))
		{
			if (const UCapsuleComponent* Capsule = Member->GetCapsuleComponent())
			{
				Radius = FMath::Max(Radius, Capsule->GetScaledCapsuleRadius());
			}
		}

		return FMath::Max(1.f, Radius * FMath::Max(0.1f, RadiusScale));
	}

	FVector GetDeterministicPairDirection(int32 A, int32 B)
	{
		const float Angle = (float)(B - A) * 2.39996323f;
		return FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
	}

	void ApplyPairCorrection(
		FPBMADSlotCorrectionMember& A,
		FPBMADSlotCorrectionMember& B,
		const FVector& DirectionFromAToB,
		float Overlap,
		float Strength
	)
	{
		if (Overlap <= 0.f || Strength <= 0.f)
		{
			return;
		}

		const float MobilitySum = FMath::Max(A.Mobility + B.Mobility, KINDA_SMALL_NUMBER);
		const FVector Correction = DirectionFromAToB.GetSafeNormal() * Overlap * Strength;

		A.CorrectedPosition -= Correction * (A.Mobility / MobilitySum);
		B.CorrectedPosition += Correction * (B.Mobility / MobilitySum);
	}

	void ClampToOriginalSlots(TArray<FPBMADSlotCorrectionMember>& Members, float MaxOffset)
	{
		for (FPBMADSlotCorrectionMember& Member : Members)
		{
			FVector Offset = Flatten(Member.CorrectedPosition - Member.OriginalPosition);
			const float AllowedOffset = MaxOffset * Member.MaxOffsetScale;
			if (Offset.SizeSquared() > AllowedOffset * AllowedOffset)
			{
				Offset = Offset.GetSafeNormal() * AllowedOffset;
				Member.CorrectedPosition.X = Member.OriginalPosition.X + Offset.X;
				Member.CorrectedPosition.Y = Member.OriginalPosition.Y + Offset.Y;
			}

			Member.CorrectedPosition.Z = Member.OriginalPosition.Z;
		}
	}

	void UpdatePredictedPositions(TArray<FPBMADSlotCorrectionMember>& Members, const FPBMADSlotCorrectionSettings& Settings)
	{
		const float PredictionTime = FMath::Max(0.05f, Settings.PredictionTime);
		const float Blend = FMath::Clamp(Settings.PreferredVelocityBlend, 0.f, 1.f);

		for (FPBMADSlotCorrectionMember& Member : Members)
		{
			const FVector ToSlot = Flatten(Member.CorrectedPosition - Member.CurrentLocation);
			Member.DesiredDirection = ToSlot.GetSafeNormal();

			const float DesiredSpeed = FMath::Clamp(ToSlot.Size() / PredictionTime, 0.f, 650.f);
			const FVector DesiredVelocity = Member.DesiredDirection * DesiredSpeed;
			const FVector BlendedVelocity = FMath::Lerp(Flatten(Member.CurrentVelocity), DesiredVelocity, Blend);
			Member.PredictedPosition = Member.CurrentLocation + BlendedVelocity * PredictionTime;
			Member.PredictedPosition.Z = Member.CurrentLocation.Z;
		}
	}

	void ApplyShortRangeConstraints(TArray<FPBMADSlotCorrectionMember>& Members, const FPBMADSlotCorrectionSettings& Settings)
	{
		for (int32 i = 0; i < Members.Num(); ++i)
		{
			for (int32 j = i + 1; j < Members.Num(); ++j)
			{
				FVector Delta = Flatten(Members[j].CorrectedPosition - Members[i].CorrectedPosition);
				float Distance = Delta.Size();
				const float RequiredDistance = FMath::Max(Settings.MinDistance, Members[i].CollisionRadius + Members[j].CollisionRadius);

				if (Distance >= RequiredDistance)
				{
					continue;
				}

				if (Distance <= KINDA_SMALL_NUMBER)
				{
					Delta = Flatten(Members[j].OriginalPosition - Members[i].OriginalPosition);
					Distance = Delta.Size();
				}

				if (Distance <= KINDA_SMALL_NUMBER)
				{
					Delta = GetDeterministicPairDirection(i, j);
				}

				ApplyPairCorrection(Members[i], Members[j], Delta, RequiredDistance - Distance, Settings.Strength);
			}
		}
	}

	void ApplyPredictiveConstraints(
		TArray<FPBMADSlotCorrectionMember>& Members,
		const FPBMADSlotCorrectionSettings& Settings,
		TArray<FIntPoint>& OutDebugPredictionPairs
	)
	{
		const float PredictionTime = FMath::Max(0.05f, Settings.PredictionTime);

		for (int32 i = 0; i < Members.Num(); ++i)
		{
			for (int32 j = i + 1; j < Members.Num(); ++j)
			{
				const FVector StartDelta = Flatten(Members[j].CurrentLocation - Members[i].CurrentLocation);
				const FVector VelA = Flatten(Members[i].PredictedPosition - Members[i].CurrentLocation) / PredictionTime;
				const FVector VelB = Flatten(Members[j].PredictedPosition - Members[j].CurrentLocation) / PredictionTime;
				const FVector RelativeVelocity = VelB - VelA;
				const float RelativeSpeedSq = RelativeVelocity.SizeSquared();

				float TimeToClosest = 0.f;
				if (RelativeSpeedSq > KINDA_SMALL_NUMBER)
				{
					TimeToClosest = FMath::Clamp(-FVector::DotProduct(StartDelta, RelativeVelocity) / RelativeSpeedSq, 0.f, PredictionTime);
				}

				FVector ClosestDelta = StartDelta + RelativeVelocity * TimeToClosest;
				float ClosestDistance = ClosestDelta.Size();
				const float RequiredDistance = FMath::Max(Settings.MinDistance, Members[i].CollisionRadius + Members[j].CollisionRadius);

				if (ClosestDistance >= RequiredDistance)
				{
					continue;
				}

				if (ClosestDistance <= KINDA_SMALL_NUMBER)
				{
					ClosestDelta = Flatten(Members[j].CorrectedPosition - Members[i].CorrectedPosition);
					ClosestDistance = ClosestDelta.Size();
				}

				if (ClosestDistance <= KINDA_SMALL_NUMBER)
				{
					ClosestDelta = GetDeterministicPairDirection(i, j);
				}

				OutDebugPredictionPairs.Add(FIntPoint(i, j));
				ApplyPairCorrection(
					Members[i],
					Members[j],
					ClosestDelta,
					(RequiredDistance - ClosestDistance) * 0.5f,
					Settings.Strength
				);
			}
		}
	}

	void ApplyShapePreservation(TArray<FPBMADSlotCorrectionMember>& Members, float ShapePreservationStrength)
	{
		const float BaseAlpha = FMath::Clamp(ShapePreservationStrength, 0.f, 1.f);
		if (BaseAlpha <= KINDA_SMALL_NUMBER)
		{
			return;
		}

		for (FPBMADSlotCorrectionMember& Member : Members)
		{
			const float Alpha = FMath::Clamp(BaseAlpha * Member.ShapePreservation, 0.f, 0.75f);
			Member.CorrectedPosition = FMath::Lerp(Member.CorrectedPosition, Member.OriginalPosition, Alpha);
		}
	}

	void ApplyCohesion(TArray<FPBMADSlotCorrectionMember>& Members, float CohesionStrength)
	{
		const float Alpha = FMath::Clamp(CohesionStrength, 0.f, 0.5f);
		if (Alpha <= KINDA_SMALL_NUMBER || Members.Num() == 0)
		{
			return;
		}

		FVector OriginalCentroid = FVector::ZeroVector;
		FVector CorrectedCentroid = FVector::ZeroVector;
		for (const FPBMADSlotCorrectionMember& Member : Members)
		{
			OriginalCentroid += Member.OriginalPosition;
			CorrectedCentroid += Member.CorrectedPosition;
		}

		OriginalCentroid /= Members.Num();
		CorrectedCentroid /= Members.Num();

		const FVector TranslationBackToShape = Flatten(OriginalCentroid - CorrectedCentroid) * Alpha;
		for (FPBMADSlotCorrectionMember& Member : Members)
		{
			Member.CorrectedPosition += TranslationBackToShape * Member.Mobility;
		}
	}

	void ProjectSlotsToNavigation(UWorld* World, TArray<FPBMADSlotCorrectionMember>& Members, const FPBMADSlotCorrectionSettings& Settings)
	{
		if (!Settings.bProjectCorrectedSlotsToNavMesh || !World)
		{
			return;
		}

		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
		if (!NavSystem)
		{
			return;
		}

		const FVector ProjectionExtent(
			FMath::Max(100.f, Settings.MinDistance * 0.5f),
			FMath::Max(100.f, Settings.MinDistance * 0.5f),
			300.f
		);

		for (FPBMADSlotCorrectionMember& Member : Members)
		{
			FNavLocation ProjectedLocation;
			if (NavSystem->ProjectPointToNavigation(Member.CorrectedPosition, ProjectedLocation, ProjectionExtent))
			{
				Member.CorrectedPosition = ProjectedLocation.Location;
				continue;
			}

			if (NavSystem->ProjectPointToNavigation(Member.OriginalPosition, ProjectedLocation, ProjectionExtent))
			{
				Member.CorrectedPosition = ProjectedLocation.Location;
			}
		}
	}

	void DrawDebugInfo(UWorld* World, const TArray<FPBMADSlotCorrectionMember>& Members, const FPBMADSlotCorrectionSettings& Settings, const TArray<FIntPoint>& PredictionPairs)
	{
		if (!World || (!Settings.bDrawDebug && !Settings.bDrawRadii && !Settings.bDrawPredictionLinks))
		{
			return;
		}

		for (int32 i = 0; i < Members.Num(); ++i)
		{
			const FPBMADSlotCorrectionMember& Member = Members[i];
			const FColor Color = Member.bIsLeader ? FColor(128, 0, 128) : FColor::Cyan;

			if (Settings.bDrawDebug)
			{
				DrawDebugLine(World, Member.OriginalPosition, Member.CorrectedPosition, Color, false, 2.f, 0, 2.f);
				DrawDebugSphere(World, Member.CorrectedPosition, 18.f, 8, Color, false, 2.f);
				DrawDebugLine(World, Member.CurrentLocation, Member.PredictedPosition, FColor::Yellow, false, 2.f, 0, 1.f);
			}

			if (Settings.bDrawRadii)
			{
				DrawDebugCircle(
					World,
					Member.CorrectedPosition,
					Member.CollisionRadius,
					24,
					Color,
					false,
					2.f,
					0,
					1.f,
					FVector::ForwardVector,
					FVector::RightVector,
					false
				);
			}
		}

		if (Settings.bDrawPredictionLinks)
		{
			for (const FIntPoint& Pair : PredictionPairs)
			{
				if (!Members.IsValidIndex(Pair.X) || !Members.IsValidIndex(Pair.Y))
				{
					continue;
				}

				DrawDebugLine(
					World,
					Members[Pair.X].PredictedPosition,
					Members[Pair.Y].PredictedPosition,
					FColor::Orange,
					false,
					2.f,
					0,
					2.f
				);
			}
		}
	}
}

void FFormationSlotCorrectionSolver::CorrectSlots(
	UWorld* World,
	const FPBMADSlotCorrectionSettings& Settings,
	TArray<FPBMADSlotCorrectionMember>& Members
)
{
	if (Members.Num() < 2)
	{
		return;
	}

	const int32 Iterations = FMath::Clamp(Settings.Iterations, 0, 16);
	if (Iterations <= 0 || Settings.MinDistance <= KINDA_SMALL_NUMBER || Settings.Strength <= KINDA_SMALL_NUMBER || Settings.MaxOffset <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	for (FPBMADSlotCorrectionMember& Member : Members)
	{
		Member.Mobility = GetRoleMobility(Member.Role, Member.bIsLeader, Settings.bEnableRoleAwareSlotCorrection);
		Member.ShapePreservation = GetRoleShapePreservation(Member.Role, Member.bIsLeader, Settings.bEnableRoleAwareSlotCorrection);
		Member.MaxOffsetScale = GetRoleMaxOffsetScale(Member.Role, Member.bIsLeader, Settings.bEnableRoleAwareSlotCorrection);
		Member.CollisionRadius = GetMemberCollisionRadius(Member.Member, Settings.MinDistance * 0.5f, Settings.RadiusScale);
	}

	TArray<FIntPoint> DebugPredictionPairs;
	for (int32 Iteration = 0; Iteration < Iterations; ++Iteration)
	{
		UpdatePredictedPositions(Members, Settings);
		ApplyShortRangeConstraints(Members, Settings);

		if (Settings.bEnablePredictiveSlotAvoidance)
		{
			ApplyPredictiveConstraints(Members, Settings, DebugPredictionPairs);
		}

		ApplyShapePreservation(Members, Settings.ShapePreservationStrength);

		if (Settings.bEnableSlotCorrectionCohesion)
		{
			ApplyCohesion(Members, Settings.CohesionStrength);
		}

		ClampToOriginalSlots(Members, Settings.MaxOffset);
	}

	UpdatePredictedPositions(Members, Settings);
	ProjectSlotsToNavigation(World, Members, Settings);
	DrawDebugInfo(World, Members, Settings, DebugPredictionPairs);
}
