#pragma once

#include "CoreMinimal.h"
#include "FormationDecision.h"
#include "NeuroFuzzyTacticalModel.generated.h"

UENUM(BlueprintType)
enum class ENeuroFuzzyTacticalInput : uint8
{
	DistanceToTarget        UMETA(DisplayName = "Distance To Target"),
	AverageGroupHealth     UMETA(DisplayName = "Average Group Health"),
	LowestMemberHealth     UMETA(DisplayName = "Lowest Member Health"),
	TargetHealth           UMETA(DisplayName = "Target Health"),
	GroupSize              UMETA(DisplayName = "Group Size"),
	TargetGroupSize        UMETA(DisplayName = "Target Group Size"),
	CombatPressure         UMETA(DisplayName = "Combat Pressure"),
	GroupSpread            UMETA(DisplayName = "Group Spread"),
	TargetMovement         UMETA(DisplayName = "Target Movement"),
	TargetIsEnemyGroup     UMETA(DisplayName = "Target Is Enemy Group")
};

UENUM(BlueprintType)
enum class ENeuroFuzzyMembershipSet : uint8
{
	Low          UMETA(DisplayName = "Low"),
	Medium       UMETA(DisplayName = "Medium"),
	High         UMETA(DisplayName = "High"),
	BooleanFalse UMETA(DisplayName = "False"),
	BooleanTrue  UMETA(DisplayName = "True")
};

USTRUCT(BlueprintType)
struct FNeuroFuzzyCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENeuroFuzzyTacticalInput Input = ENeuroFuzzyTacticalInput::DistanceToTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENeuroFuzzyMembershipSet MembershipSet = ENeuroFuzzyMembershipSet::Medium;
};

USTRUCT(BlueprintType)
struct FNeuroFuzzyRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RuleId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTargetsTuningPreset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFormationType FormationOutput = EFormationType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFormationTuningPreset TuningOutput = EFormationTuningPreset::Balanced;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Consequent = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FNeuroFuzzyCondition> Conditions;
};

USTRUCT(BlueprintType)
struct FNeuroFuzzyOutputScores
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float CircleFormationScore = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float LineFormationScore = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float WedgeFormationScore = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float CompactTuningScore = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float BalancedTuningScore = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float SpreadTuningScore = 0.f;
};
