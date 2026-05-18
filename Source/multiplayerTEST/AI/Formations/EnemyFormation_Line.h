// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "multiplayerTEST/AI/Formations/EnemyFormation.h"
#include "EnemyFormation_Line.generated.h"

UCLASS()
class MULTIPLAYERTEST_API UEnemyFormation_Line : public UEnemyFormation
{
	GENERATED_BODY()
protected:

	UPROPERTY()
	FVector CachedDirFromTarget = FVector::ZeroVector;

	UPROPERTY()
	float DesiredDistanceToTarget = 700.f;

	UPROPERTY()
	float DistanceTolerance = 50.f;

	UPROPERTY()
	float DirectionUpdateThreshold = 0.1f; // dot product threshold (~6 degrees)
public:
	virtual void UpdateFormationAnchor(
		AEnemyCore* Leader,
		AActor* Target,
		const TArray<AEnemyCore*>& Members
	) override;

    virtual FVector GetLocationForMember(
		AEnemyCore* Member,
		int32 Index,
		int32 TotalMembers,
		AActor* TargetActor
    ) const;

	// Compatibility overload: allow callers to pass Leader explicitly if needed.
	FVector GetLocationForMember(AEnemyCore* Leader, AEnemyCore* Member, int32 Index, int32 TotalMembers, AActor* TargetActor) const;

	// Minimum spacing between members on a single row (world units)
	UPROPERTY(EditAnywhere)
	float MinSpacing = 150.f;

	// Maximum usable length for a single row. If more members would exceed this
	// length they will flow into the next row.
	UPROPERTY(EditAnywhere)
	float MaxLineLength = 10000.f;

	// Distance to offset subsequent rows behind the first one.
	UPROPERTY(EditAnywhere)
	float RowOffset = 200.f;

	// Leader offset distance behind the formation anchor
	UPROPERTY(EditAnywhere)
	float LeaderOffset = 300.f;
};
