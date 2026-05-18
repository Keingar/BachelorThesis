// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFormation_Circle.h"
#include "multiplayerTEST/Characters/Enemies/EnemyCore.h"

namespace
{
	FVector GetStaggeredForwardArcLocation(
		const FVector& AnchorLocation,
		const FVector& Forward,
		float Radius,
		float LayerRadiusOffset,
		float ArcAngleDegrees,
		float ArcOffsetDegrees,
		float MinAnglePerMemberDeg,
		int32 Index,
		int32 TotalMembers
	)
	{
		if (TotalMembers <= 0)
		{
			return FVector::ZeroVector;
		}

		const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
		const bool bFullCircle = ArcAngleDegrees >= 300.f;
		const float FrontArcDegrees = bFullCircle
			? 360.f
			: FMath::Clamp(ArcAngleDegrees, MinAnglePerMemberDeg, 300.f);
		const float SlotSpacingDegrees = FMath::Max(MinAnglePerMemberDeg, 1.f);
		const int32 SlotsPerLayer = FMath::Max(1, FMath::CeilToInt(FrontArcDegrees / SlotSpacingDegrees));
		const int32 Layer = Index / SlotsPerLayer;
		const int32 IndexInLayer = Index % SlotsPerLayer;
		const int32 MembersOnLayer = FMath::Min(SlotsPerLayer, TotalMembers - Layer * SlotsPerLayer);
		const float ArcRad = FMath::DegreesToRadians(FrontArcDegrees);
		float Angle = FMath::DegreesToRadians(ArcOffsetDegrees);

		if (bFullCircle)
		{
			const float Step = (2.f * PI) / FMath::Max(1, MembersOnLayer);
			Angle += IndexInLayer * Step;
			if (Layer % 2 == 1)
			{
				Angle += Step * 0.5f;
			}
		}
		else if (MembersOnLayer > 1)
		{
			const float Step = ArcRad / MembersOnLayer;
			Angle += -ArcRad * 0.5f + Step * 0.5f + IndexInLayer * Step;
		}

		const float UsedRadius = Radius + Layer * LayerRadiusOffset;
		const FVector Dir = Forward * FMath::Cos(Angle) + Right * FMath::Sin(Angle);

		return AnchorLocation + Dir * UsedRadius;
	}
}


void UEnemyFormation_Circle::UpdateFormationAnchor(
	AEnemyCore* Leader,
	AActor* Target,
	const TArray<AEnemyCore*>& Members
)
{
	if (!Target && !HasTargetLocationOverride()) return;

	// Anchor is ALWAYS target-based
	AnchorLocation = ResolveTargetLocation(Target);

	// Only set ONCE (no per-frame drift)
	if (!bHasValidAnchor)
	{
		// stable world orientation OR initial direction from target->leader
		if (Leader)
		{
			FVector Dir = (Leader->GetActorLocation() - AnchorLocation).GetSafeNormal();
			FixedForward = FVector::VectorPlaneProject(Dir, FVector::UpVector).GetSafeNormal();
			if (FixedForward.IsNearlyZero())
			{
				FixedForward = FVector(1, 0, 0);
			}
		}
		else
		{
			FixedForward = FVector(1, 0, 0);
		}

		bHasValidAnchor = true;
	}

	AnchorForward = FixedForward;

	EffectiveArcAngleDegrees = FMath::Clamp(ArcAngleDegrees, MinAnglePerMemberDeg, 360.f);
}

FVector UEnemyFormation_Circle::GetLocationForMember(
    AEnemyCore* Member,
    int32 Index,
    int32 TotalMembers,
    AActor* TargetActor
) const
{
    if (TotalMembers <= 0)
        return FVector::ZeroVector;

    FVector Forward = FixedForward;

    // Leader special case: place leader behind the circle at LeaderOffset distance
    if (Member && CurrentLeader && Member == CurrentLeader)
    {
        // choose radial direction towards current leader position to minimize travel
        FVector ToLeader = FVector::VectorPlaneProject(CurrentLeader->GetActorLocation() - AnchorLocation, FVector::UpVector);
        FVector RadDir = ToLeader.IsNearlyZero() ? (-Forward) : ToLeader.GetSafeNormal();
        float LeaderRadius = Radius + LeaderOffset;
        return AnchorLocation + RadDir * LeaderRadius;
    }

	const bool bHasLeaderSlot = CurrentLeader != nullptr && TotalMembers > 1;
	const int32 FormationIndex = bHasLeaderSlot && Index > 0 ? Index - 1 : Index;
	const int32 FormationTotal = bHasLeaderSlot ? TotalMembers - 1 : TotalMembers;

	return GetStaggeredForwardArcLocation(
		AnchorLocation,
		Forward,
		Radius,
		LayerRadiusOffset,
		EffectiveArcAngleDegrees,
		ArcOffsetDegrees,
		MinAnglePerMemberDeg,
		FormationIndex,
		FormationTotal
	);
}

// Compatibility overload: accept Leader explicitly and compute using it instead of CurrentLeader
FVector UEnemyFormation_Circle::GetLocationForMember(AEnemyCore* Leader, AEnemyCore* Member, int32 Index, int32 TotalMembers, AActor* TargetActor) const
{
    if (TotalMembers <= 0)
        return FVector::ZeroVector;

    FVector Forward = FixedForward;

    if (Member && Leader && Member == Leader)
    {
        FVector ToLeader = FVector::VectorPlaneProject(Leader->GetActorLocation() - AnchorLocation, FVector::UpVector);
        FVector RadDir = ToLeader.IsNearlyZero() ? (-Forward) : ToLeader.GetSafeNormal();
        float LeaderRadius = Radius + LeaderOffset;
        return AnchorLocation + RadDir * LeaderRadius;
    }

	const bool bHasLeaderSlot = Leader != nullptr && TotalMembers > 1;
	const int32 FormationIndex = bHasLeaderSlot && Index > 0 ? Index - 1 : Index;
	const int32 FormationTotal = bHasLeaderSlot ? TotalMembers - 1 : TotalMembers;

	return GetStaggeredForwardArcLocation(
		AnchorLocation,
		Forward,
		Radius,
		LayerRadiusOffset,
		EffectiveArcAngleDegrees,
		ArcOffsetDegrees,
		MinAnglePerMemberDeg,
		FormationIndex,
		FormationTotal
	);
}
