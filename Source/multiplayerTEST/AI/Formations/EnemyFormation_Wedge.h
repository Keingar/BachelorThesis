// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "multiplayerTEST/AI/Formations/EnemyFormation.h"
#include "EnemyFormation_Wedge.generated.h"

UCLASS()
class MULTIPLAYERTEST_API UEnemyFormation_Wedge : public UEnemyFormation
{
	GENERATED_BODY()

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
	) const override;

	FVector GetLocationForMember(AEnemyCore* Leader, AEnemyCore* Member, int32 Index, int32 TotalMembers, AActor* TargetActor) const;

	UPROPERTY(EditAnywhere)
	float DesiredDistanceToTarget = 700.f;

	UPROPERTY(EditAnywhere)
	float SideSpacing = 180.f;

	UPROPERTY(EditAnywhere)
	float DepthSpacing = 220.f;

	UPROPERTY(EditAnywhere)
	float LeaderOffset = 300.f;
};
