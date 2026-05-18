// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/NoExportTypes.h"
#include "EnemyFormation.generated.h"

class AEnemyCore;

UCLASS(Abstract, Blueprintable)
class MULTIPLAYERTEST_API UEnemyFormation : public UObject
{
	GENERATED_BODY()

public:
	virtual void UpdateFormationAnchor(
		AEnemyCore* Leader,
		AActor* Target,
		const TArray<AEnemyCore*>& Members
	) {
	}

	virtual FVector GetLocationForMember(
		AEnemyCore* Member,
		int32 Index,
		int32 TotalMembers,
		AActor* TargetActor
	) const PURE_VIRTUAL(UEnemyFormation::GetLocationForMember, return FVector::ZeroVector;);

	// Set the current leader for formations to use when computing member slots.
	void SetLeader(AEnemyCore* NewLeader) { CurrentLeader = NewLeader; }

	void SetTargetLocationOverride(const FVector& NewTargetLocation, bool bEnableOverride = true)
	{
		TargetLocationOverride = NewTargetLocation;
		bUseTargetLocationOverride = bEnableOverride;
	}

	void ResetFormation()
	{
		bHasValidAnchor = false;
	}

	// Expose anchor info to callers (read-only)
	FVector GetAnchorLocation() const { return AnchorLocation; }
	FVector GetAnchorForward() const { return AnchorForward; }
	bool HasValidAnchor() const { return bHasValidAnchor; }
	bool HasTargetLocationOverride() const { return bUseTargetLocationOverride; }

protected:
	FVector ResolveTargetLocation(AActor* TargetActor) const
	{
		if (bUseTargetLocationOverride)
		{
			return TargetLocationOverride;
		}

		return TargetActor ? TargetActor->GetActorLocation() : FVector::ZeroVector;
	}

	UPROPERTY()
	FVector AnchorLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector AnchorForward = FVector::ForwardVector;

	UPROPERTY()
	bool bHasValidAnchor = false;

	// Current leader pointer (set by owner before UpdateFormationAnchor)
	UPROPERTY()
	AEnemyCore* CurrentLeader = nullptr;

	UPROPERTY()
	FVector TargetLocationOverride = FVector::ZeroVector;

	UPROPERTY()
	bool bUseTargetLocationOverride = false;
};
