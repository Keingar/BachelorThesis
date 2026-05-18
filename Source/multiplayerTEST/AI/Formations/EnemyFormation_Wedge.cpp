#include "EnemyFormation_Wedge.h"
#include "multiplayerTEST/Characters/Enemies/EnemyCore.h"

namespace
{
	FVector GetWedgeSlotLocation(
		const FVector& AnchorLocation,
		const FVector& AnchorForward,
		float SideSpacing,
		float DepthSpacing,
		int32 FormationIndex
	)
	{
		if (FormationIndex <= 0)
		{
			return AnchorLocation;
		}

		const FVector Right = FVector::CrossProduct(FVector::UpVector, AnchorForward).GetSafeNormal();
		const int32 PairIndex = (FormationIndex + 1) / 2;
		const float SideSign = (FormationIndex % 2 == 1) ? -1.f : 1.f;

		return AnchorLocation
			+ Right * SideSign * PairIndex * SideSpacing
			- AnchorForward * PairIndex * DepthSpacing;
	}
}

void UEnemyFormation_Wedge::UpdateFormationAnchor(
	AEnemyCore* Leader,
	AActor* Target,
	const TArray<AEnemyCore*>& Members
)
{
	if ((!Target && !HasTargetLocationOverride()) || Members.Num() == 0) return;

	FVector AvgLocation = FVector::ZeroVector;
	int32 ValidCount = 0;

	for (AEnemyCore* Member : Members)
	{
		if (!IsValid(Member)) continue;

		AvgLocation += Member->GetActorLocation();
		ValidCount++;
	}

	if (ValidCount == 0) return;

	AvgLocation /= ValidCount;

	const FVector TargetPos = ResolveTargetLocation(Target);
	FVector DirToTarget = FVector::VectorPlaneProject(TargetPos - AvgLocation, FVector::UpVector).GetSafeNormal();
	if (DirToTarget.IsNearlyZero())
	{
		DirToTarget = FVector::ForwardVector;
	}

	AnchorForward = DirToTarget;
	AnchorLocation = TargetPos - AnchorForward * DesiredDistanceToTarget;
	bHasValidAnchor = true;
}

FVector UEnemyFormation_Wedge::GetLocationForMember(
	AEnemyCore* Member,
	int32 Index,
	int32 TotalMembers,
	AActor* TargetActor
) const
{
	if (TotalMembers <= 0)
	{
		return FVector::ZeroVector;
	}

	if (Member && CurrentLeader && Member == CurrentLeader)
	{
		return AnchorLocation - AnchorForward * LeaderOffset;
	}

	const bool bHasLeaderSlot = CurrentLeader != nullptr && TotalMembers > 1;
	const int32 FormationIndex = bHasLeaderSlot && Index > 0 ? Index - 1 : Index;

	return GetWedgeSlotLocation(AnchorLocation, AnchorForward, SideSpacing, DepthSpacing, FormationIndex);
}

FVector UEnemyFormation_Wedge::GetLocationForMember(AEnemyCore* Leader, AEnemyCore* Member, int32 Index, int32 TotalMembers, AActor* TargetActor) const
{
	if (TotalMembers <= 0)
	{
		return FVector::ZeroVector;
	}

	if (Member && Leader && Member == Leader)
	{
		return AnchorLocation - AnchorForward * LeaderOffset;
	}

	const bool bHasLeaderSlot = Leader != nullptr && TotalMembers > 1;
	const int32 FormationIndex = bHasLeaderSlot && Index > 0 ? Index - 1 : Index;

	return GetWedgeSlotLocation(AnchorLocation, AnchorForward, SideSpacing, DepthSpacing, FormationIndex);
}
