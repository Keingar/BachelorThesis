#pragma once

#include "CoreMinimal.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/GroupCombatContext.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/NeuroFuzzyTacticalModel.h"

struct FNeuroFuzzyRuleActivation
{
	FNeuroFuzzyRule Rule;
	float FiringStrength = 0.f;
};

class MULTIPLAYERTEST_API FEnemyNeuroFuzzyEvaluator
{
public:
	static FNeuroFuzzyOutputScores Evaluate(const FGroupCombatContext& Context, const TMap<FName, float>& LearnedConsequentOffsets);
	static TArray<FNeuroFuzzyRuleActivation> EvaluateRuleActivations(const FGroupCombatContext& Context);
	static TArray<FNeuroFuzzyRule> BuildDefaultRules();

private:
	static float GetMembershipValue(const FGroupCombatContext& Context, ENeuroFuzzyTacticalInput Input, ENeuroFuzzyMembershipSet Set);
	static float GetRuleFiringStrength(const FGroupCombatContext& Context, const FNeuroFuzzyRule& Rule);
	static float GetPressureValue(const FGroupCombatContext& Context);
	static float RampUp(float Value, float Min, float Max);
	static float RampDown(float Value, float Min, float Max);
	static float Triangle(float Value, float Min, float Peak, float Max);
	static void AccumulateOutputScore(
		FNeuroFuzzyOutputScores& Scores,
		EFormationType FormationOutput,
		EFormationTuningPreset TuningOutput,
		bool bTargetsTuningPreset,
		float FiringStrength,
		float WeightedConsequent,
		float& CircleWeight,
		float& LineWeight,
		float& WedgeWeight,
		float& CompactWeight,
		float& BalancedWeight,
		float& SpreadWeight
	);
	static void NormalizeOutputScores(
		FNeuroFuzzyOutputScores& Scores,
		float CircleWeight,
		float LineWeight,
		float WedgeWeight,
		float CompactWeight,
		float BalancedWeight,
		float SpreadWeight
	);
};
