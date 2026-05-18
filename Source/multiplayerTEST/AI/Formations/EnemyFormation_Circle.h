// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "multiplayerTEST/AI/Formations/EnemyFormation.h"
#include "EnemyFormation_Circle.generated.h"

UCLASS()
class MULTIPLAYERTEST_API UEnemyFormation_Circle : public UEnemyFormation
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float Radius = 700.f;

	UPROPERTY(EditAnywhere)
	float ArcAngleDegrees = 360.f;

	UPROPERTY(EditAnywhere)
	float ArcOffsetDegrees = 0.f;

	UPROPERTY(EditAnywhere)
	float MinAnglePerMemberDeg = 10;

	UPROPERTY(EditAnywhere)
	float LayerRadiusOffset = 200.f;

	UPROPERTY(EditAnywhere)
	float LeaderOffset = 300.f;

	// FIXED orientation (no drift)
	FVector FixedForward = FVector(1, 0, 0);

	UPROPERTY(EditAnywhere, meta = (ClampMax = "300"))
	float MaxFullCircleMemberSpacing = 200.f;

	UPROPERTY(EditAnywhere)
	int32 MinMembersForFullCircle = 4;

	UPROPERTY()
	float EffectiveArcAngleDegrees = 360.f;

	virtual void UpdateFormationAnchor(
		AEnemyCore *Leader,
		AActor *Target,
		const TArray<AEnemyCore *> &Members) override;

	virtual FVector GetLocationForMember(
		AEnemyCore *Member,
		int32 Index,
		int32 TotalMembers,
		AActor *TargetActor) const;

	FVector GetLocationForMember(AEnemyCore *Leader, AEnemyCore *Member, int32 Index, int32 TotalMembers, AActor *TargetActor) const;
};
