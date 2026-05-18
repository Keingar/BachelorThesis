#pragma once

#include "CoreMinimal.h"
#include "GroupCombatContext.generated.h"

USTRUCT(BlueprintType)
struct FGroupCombatContext
{
	GENERATED_BODY()

public:
    // Basic
	UPROPERTY(BlueprintReadWrite)
	int32 GroupSize = 0;

	UPROPERTY(BlueprintReadWrite)
	AActor* Target = nullptr;

	UPROPERTY(BlueprintReadWrite)
	bool bTargetIsEnemyGroup = false;

	UPROPERTY(BlueprintReadWrite)
	int32 TargetGroupSize = 0;

	UPROPERTY(BlueprintReadWrite)
	FVector TargetGroupCenter = FVector::ZeroVector;

	// Distance from formation anchor (or leader) to target
	UPROPERTY(BlueprintReadWrite)
	float DistanceToTarget = 0.f;

	// Health / combat
	// Average group health in normalized 0..1 (1 == full health)
	UPROPERTY(BlueprintReadWrite)
	float AverageGroupHealth = 1.f;

	// Lowest member health normalized 0..1
	UPROPERTY(BlueprintReadWrite)
	float LowestMemberHealth = 1.f;

	// Target health normalized 0..1
	UPROPERTY(BlueprintReadWrite)
	float TargetHealth = 1.f;

	// Kinematics
	UPROPERTY(BlueprintReadWrite)
	float TargetVelocity = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float DamageTakenRecently = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float DamageDealtRecently = 0.f;

	// Formation / spread
	UPROPERTY(BlueprintReadWrite)
	float AverageDistanceBetweenMembers = 0.f;

	// Useful tactical booleans
	UPROPERTY(BlueprintReadWrite)
	bool bTargetInsideFormation = false;

	UPROPERTY(BlueprintReadWrite)
	bool bGroupSpreadTooFar = false;

	UPROPERTY(BlueprintReadWrite)
	bool bTargetEscaping = false;

	UPROPERTY(BlueprintReadWrite)
	bool bTakingHeavyLosses = false;

};
