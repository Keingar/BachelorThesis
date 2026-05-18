#include "EnemyFormation_Line.h"
#include "multiplayerTEST/Characters/Enemies/EnemyCore.h"

void UEnemyFormation_Line::UpdateFormationAnchor(
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

	FVector TargetPos = ResolveTargetLocation(Target);

	FVector NewDir = FVector::VectorPlaneProject(TargetPos - AvgLocation, FVector::UpVector).GetSafeNormal();
	if (NewDir.IsNearlyZero())
	{
		NewDir = FVector::ForwardVector;
	}

	FVector CurrentCenter = bHasValidAnchor ? AnchorLocation : AvgLocation;

	float CurrentDistance = FVector::Dist(CurrentCenter, TargetPos);

	bool bShouldUpdate = false;

	// First init
	if (!bHasValidAnchor)
	{
		bShouldUpdate = true;
		bHasValidAnchor = true;
	}
	else
	{
		// Distance check
		if (FMath::Abs(CurrentDistance - DesiredDistanceToTarget) > DistanceTolerance)
		{
			bShouldUpdate = true;
		}

		// Direction check
		float Dot = FVector::DotProduct(NewDir, CachedDirFromTarget);
		if (Dot < (1.0f - DirectionUpdateThreshold))
		{
			bShouldUpdate = true;
		}
	}

	if (bShouldUpdate)
	{
		CachedDirFromTarget = NewDir;

		AnchorLocation = TargetPos - CachedDirFromTarget * DesiredDistanceToTarget;

		AnchorForward = CachedDirFromTarget;
	}
}


FVector UEnemyFormation_Line::GetLocationForMember(
	AEnemyCore* Member,
	int32 Index,
	int32 TotalMembers,
	AActor* TargetActor
) const
{
	if (TotalMembers <= 0)
		return FVector::ZeroVector;

	FVector Right = FVector::CrossProduct(FVector::UpVector, AnchorForward).GetSafeNormal();

	// Leader special case: place leader centered behind the formation.
	if (Member && CurrentLeader && Member == CurrentLeader)
	{
		return AnchorLocation - AnchorForward * LeaderOffset;
	}

	const bool bHasLeaderSlot = CurrentLeader != nullptr && TotalMembers > 1;
	const int32 FormationIndex = bHasLeaderSlot && Index > 0 ? Index - 1 : Index;
	const int32 FormationTotal = bHasLeaderSlot ? TotalMembers - 1 : TotalMembers;

    // Determine how many slots fit per row based on MinSpacing and MaxLineLength
    const float Spacing = MinSpacing;
    int32 SlotsPerRow = FMath::Max(1, FMath::FloorToInt(MaxLineLength / Spacing));

    int32 Row = FormationIndex / SlotsPerRow;
    int32 IndexInRow = FormationIndex % SlotsPerRow;

    // Number of members on this row (last row may be partial). To center the
    // members we need the count for this row specifically.
    int32 MembersBeforeThisRow = Row * SlotsPerRow;
    int32 MembersOnThisRow = FMath::Max(1, FMath::Min(SlotsPerRow, FormationTotal - MembersBeforeThisRow));

    // Center horizontally on the row
    float Offset = (IndexInRow - (MembersOnThisRow - 1) * 0.5f) * Spacing;

	FVector RowOffsetVec = -AnchorForward * (Row * RowOffset);

	FVector SlotPos = AnchorLocation + Right * Offset + RowOffsetVec;

	return SlotPos;
}

// Compatibility overload: accept Leader explicitly and compute using it instead of CurrentLeader
FVector UEnemyFormation_Line::GetLocationForMember(AEnemyCore* Leader, AEnemyCore* Member, int32 Index, int32 TotalMembers, AActor* TargetActor) const
{
    if (TotalMembers <= 0)
        return FVector::ZeroVector;

    FVector Right = FVector::CrossProduct(FVector::UpVector, AnchorForward).GetSafeNormal();

    if (Member && Leader && Member == Leader)
    {
        return AnchorLocation - AnchorForward * LeaderOffset;
    }

    const bool bHasLeaderSlot = Leader != nullptr && TotalMembers > 1;
    const int32 FormationIndex = bHasLeaderSlot && Index > 0 ? Index - 1 : Index;
    const int32 FormationTotal = bHasLeaderSlot ? TotalMembers - 1 : TotalMembers;

    const float Spacing = MinSpacing;
    int32 SlotsPerRow = FMath::Max(1, FMath::FloorToInt(MaxLineLength / Spacing));

    int32 Row = FormationIndex / SlotsPerRow;
    int32 IndexInRow = FormationIndex % SlotsPerRow;

    int32 MembersBeforeThisRow = Row * SlotsPerRow;
    int32 MembersOnThisRow = FMath::Max(1, FMath::Min(SlotsPerRow, FormationTotal - MembersBeforeThisRow));

    float Offset = (IndexInRow - (MembersOnThisRow - 1) * 0.5f) * Spacing;

    FVector RowOffsetVec = -AnchorForward * (Row * RowOffset);

    FVector SlotPos = AnchorLocation + Right * Offset + RowOffsetVec;

    return SlotPos;
}
