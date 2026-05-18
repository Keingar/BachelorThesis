#include "EnemyNeuroFuzzyEvaluator.h"

#include "Math/UnrealMathUtility.h"

namespace
{
	FNeuroFuzzyCondition MakeCondition(ENeuroFuzzyTacticalInput Input, ENeuroFuzzyMembershipSet Set)
	{
		FNeuroFuzzyCondition Condition;
		Condition.Input = Input;
		Condition.MembershipSet = Set;
		return Condition;
	}

	FNeuroFuzzyRule MakeFormationRule(FName RuleId, EFormationType Formation, float Consequent, TArray<FNeuroFuzzyCondition> Conditions)
	{
		FNeuroFuzzyRule Rule;
		Rule.RuleId = RuleId;
		Rule.bTargetsTuningPreset = false;
		Rule.FormationOutput = Formation;
		Rule.Consequent = Consequent;
		Rule.Conditions = MoveTemp(Conditions);
		return Rule;
	}

	FNeuroFuzzyRule MakeTuningRule(FName RuleId, EFormationType Formation, EFormationTuningPreset Preset, float Consequent, TArray<FNeuroFuzzyCondition> Conditions)
	{
		FNeuroFuzzyRule Rule;
		Rule.RuleId = RuleId;
		Rule.bTargetsTuningPreset = true;
		Rule.FormationOutput = Formation;
		Rule.TuningOutput = Preset;
		Rule.Consequent = Consequent;
		Rule.Conditions = MoveTemp(Conditions);
		return Rule;
	}
}

FNeuroFuzzyOutputScores FEnemyNeuroFuzzyEvaluator::Evaluate(const FGroupCombatContext& Context, const TMap<FName, float>& LearnedConsequentOffsets)
{
	FNeuroFuzzyOutputScores Scores;
	float CircleWeight = 0.f;
	float LineWeight = 0.f;
	float WedgeWeight = 0.f;
	float CompactWeight = 0.f;
	float BalancedWeight = 0.f;
	float SpreadWeight = 0.f;

	for (const FNeuroFuzzyRuleActivation& Activation : EvaluateRuleActivations(Context))
	{
		if (Activation.FiringStrength <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		const float LearnedOffset = LearnedConsequentOffsets.FindRef(Activation.Rule.RuleId);
		const float Consequent = FMath::Clamp(Activation.Rule.Consequent + LearnedOffset, 0.f, 1.f);
		const float WeightedConsequent = Activation.FiringStrength * Consequent;

		AccumulateOutputScore(
			Scores,
			Activation.Rule.FormationOutput,
			Activation.Rule.TuningOutput,
			Activation.Rule.bTargetsTuningPreset,
			Activation.FiringStrength,
			WeightedConsequent,
			CircleWeight,
			LineWeight,
			WedgeWeight,
			CompactWeight,
			BalancedWeight,
			SpreadWeight
		);
	}

	NormalizeOutputScores(Scores, CircleWeight, LineWeight, WedgeWeight, CompactWeight, BalancedWeight, SpreadWeight);
	return Scores;
}

TArray<FNeuroFuzzyRuleActivation> FEnemyNeuroFuzzyEvaluator::EvaluateRuleActivations(const FGroupCombatContext& Context)
{
	TArray<FNeuroFuzzyRuleActivation> Activations;
	for (const FNeuroFuzzyRule& Rule : BuildDefaultRules())
	{
		FNeuroFuzzyRuleActivation Activation;
		Activation.Rule = Rule;
		Activation.FiringStrength = GetRuleFiringStrength(Context, Rule);
		Activations.Add(MoveTemp(Activation));
	}
	return Activations;
}

TArray<FNeuroFuzzyRule> FEnemyNeuroFuzzyEvaluator::BuildDefaultRules()
{
	TArray<FNeuroFuzzyRule> Rules;
	Rules.Reserve(18);

	Rules.Add(MakeFormationRule(
		FName(TEXT("Formation.Circle.ClosePressure")),
		EFormationType::Circle,
		0.86f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::DistanceToTarget, ENeuroFuzzyMembershipSet::Low),
			MakeCondition(ENeuroFuzzyTacticalInput::CombatPressure, ENeuroFuzzyMembershipSet::High)
		}
	));
	Rules.Add(MakeFormationRule(
		FName(TEXT("Formation.Circle.LowHealth")),
		EFormationType::Circle,
		0.82f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::AverageGroupHealth, ENeuroFuzzyMembershipSet::Low),
			MakeCondition(ENeuroFuzzyTacticalInput::LowestMemberHealth, ENeuroFuzzyMembershipSet::Low)
		}
	));
	Rules.Add(MakeFormationRule(
		FName(TEXT("Formation.Circle.LargeCloseGroup")),
		EFormationType::Circle,
		0.74f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::GroupSize, ENeuroFuzzyMembershipSet::High),
			MakeCondition(ENeuroFuzzyTacticalInput::DistanceToTarget, ENeuroFuzzyMembershipSet::Low)
		}
	));
	Rules.Add(MakeFormationRule(
		FName(TEXT("Formation.Line.FarHealthy")),
		EFormationType::Line,
		0.88f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::DistanceToTarget, ENeuroFuzzyMembershipSet::High),
			MakeCondition(ENeuroFuzzyTacticalInput::AverageGroupHealth, ENeuroFuzzyMembershipSet::High)
		}
	));
	Rules.Add(MakeFormationRule(
		FName(TEXT("Formation.Line.FastTarget")),
		EFormationType::Line,
		0.78f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::TargetMovement, ENeuroFuzzyMembershipSet::High),
			MakeCondition(ENeuroFuzzyTacticalInput::CombatPressure, ENeuroFuzzyMembershipSet::Low)
		}
	));
	Rules.Add(MakeFormationRule(
		FName(TEXT("Formation.Line.MediumLowPressure")),
		EFormationType::Line,
		0.66f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::DistanceToTarget, ENeuroFuzzyMembershipSet::Medium),
			MakeCondition(ENeuroFuzzyTacticalInput::CombatPressure, ENeuroFuzzyMembershipSet::Low)
		}
	));
	Rules.Add(MakeFormationRule(
		FName(TEXT("Formation.Wedge.EnemyGroupHealthy")),
		EFormationType::Wedge,
		0.9f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::TargetIsEnemyGroup, ENeuroFuzzyMembershipSet::BooleanTrue),
			MakeCondition(ENeuroFuzzyTacticalInput::AverageGroupHealth, ENeuroFuzzyMembershipSet::High),
			MakeCondition(ENeuroFuzzyTacticalInput::TargetGroupSize, ENeuroFuzzyMembershipSet::High)
		}
	));
	Rules.Add(MakeFormationRule(
		FName(TEXT("Formation.Wedge.MediumGroupFight")),
		EFormationType::Wedge,
		0.78f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::TargetIsEnemyGroup, ENeuroFuzzyMembershipSet::BooleanTrue),
			MakeCondition(ENeuroFuzzyTacticalInput::DistanceToTarget, ENeuroFuzzyMembershipSet::Medium),
			MakeCondition(ENeuroFuzzyTacticalInput::CombatPressure, ENeuroFuzzyMembershipSet::Low)
		}
	));
	Rules.Add(MakeFormationRule(
		FName(TEXT("Formation.Wedge.SpreadEnemyGroup")),
		EFormationType::Wedge,
		0.68f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::TargetIsEnemyGroup, ENeuroFuzzyMembershipSet::BooleanTrue),
			MakeCondition(ENeuroFuzzyTacticalInput::GroupSpread, ENeuroFuzzyMembershipSet::Low),
			MakeCondition(ENeuroFuzzyTacticalInput::TargetGroupSize, ENeuroFuzzyMembershipSet::Medium)
		}
	));

	Rules.Add(MakeTuningRule(
		FName(TEXT("Tuning.Circle.Compact.TargetInside")),
		EFormationType::Circle,
		EFormationTuningPreset::Compact,
		0.72f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::DistanceToTarget, ENeuroFuzzyMembershipSet::Low),
			MakeCondition(ENeuroFuzzyTacticalInput::GroupSpread, ENeuroFuzzyMembershipSet::High)
		}
	));
	Rules.Add(MakeTuningRule(
		FName(TEXT("Tuning.Circle.Spread.Pressure")),
		EFormationType::Circle,
		EFormationTuningPreset::Spread,
		0.76f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::CombatPressure, ENeuroFuzzyMembershipSet::High),
			MakeCondition(ENeuroFuzzyTacticalInput::AverageGroupHealth, ENeuroFuzzyMembershipSet::Low)
		}
	));
	Rules.Add(MakeTuningRule(
		FName(TEXT("Tuning.Line.Compact.Regroup")),
		EFormationType::Line,
		EFormationTuningPreset::Compact,
		0.7f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::GroupSpread, ENeuroFuzzyMembershipSet::High),
			MakeCondition(ENeuroFuzzyTacticalInput::CombatPressure, ENeuroFuzzyMembershipSet::Low)
		}
	));
	Rules.Add(MakeTuningRule(
		FName(TEXT("Tuning.Line.Spread.HealthyFar")),
		EFormationType::Line,
		EFormationTuningPreset::Spread,
		0.72f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::DistanceToTarget, ENeuroFuzzyMembershipSet::High),
			MakeCondition(ENeuroFuzzyTacticalInput::AverageGroupHealth, ENeuroFuzzyMembershipSet::High)
		}
	));
	Rules.Add(MakeTuningRule(
		FName(TEXT("Tuning.Wedge.Compact.Press")),
		EFormationType::Wedge,
		EFormationTuningPreset::Compact,
		0.67f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::TargetIsEnemyGroup, ENeuroFuzzyMembershipSet::BooleanTrue),
			MakeCondition(ENeuroFuzzyTacticalInput::DistanceToTarget, ENeuroFuzzyMembershipSet::Medium),
			MakeCondition(ENeuroFuzzyTacticalInput::AverageGroupHealth, ENeuroFuzzyMembershipSet::High)
		}
	));
	Rules.Add(MakeTuningRule(
		FName(TEXT("Tuning.Wedge.Spread.LargeTargetGroup")),
		EFormationType::Wedge,
		EFormationTuningPreset::Spread,
		0.75f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::TargetIsEnemyGroup, ENeuroFuzzyMembershipSet::BooleanTrue),
			MakeCondition(ENeuroFuzzyTacticalInput::TargetGroupSize, ENeuroFuzzyMembershipSet::High)
		}
	));
	Rules.Add(MakeTuningRule(
		FName(TEXT("Tuning.Balanced.DefaultLowSignal")),
		EFormationType::None,
		EFormationTuningPreset::Balanced,
		0.55f,
		{
			MakeCondition(ENeuroFuzzyTacticalInput::CombatPressure, ENeuroFuzzyMembershipSet::Medium)
		}
	));

	return Rules;
}

float FEnemyNeuroFuzzyEvaluator::GetMembershipValue(const FGroupCombatContext& Context, ENeuroFuzzyTacticalInput Input, ENeuroFuzzyMembershipSet Set)
{
	if (Input == ENeuroFuzzyTacticalInput::TargetIsEnemyGroup)
	{
		const bool bTrue = Context.bTargetIsEnemyGroup && Context.TargetGroupSize >= 3;
		if (Set == ENeuroFuzzyMembershipSet::BooleanTrue)
		{
			return bTrue ? 1.f : 0.f;
		}
		if (Set == ENeuroFuzzyMembershipSet::BooleanFalse)
		{
			return bTrue ? 0.f : 1.f;
		}
		return 0.f;
	}

	float Value = 0.f;
	switch (Input)
	{
	case ENeuroFuzzyTacticalInput::DistanceToTarget:
		Value = Context.DistanceToTarget;
		break;
	case ENeuroFuzzyTacticalInput::AverageGroupHealth:
		Value = FMath::Clamp(Context.AverageGroupHealth, 0.f, 1.f);
		break;
	case ENeuroFuzzyTacticalInput::LowestMemberHealth:
		Value = FMath::Clamp(Context.LowestMemberHealth, 0.f, 1.f);
		break;
	case ENeuroFuzzyTacticalInput::TargetHealth:
		Value = FMath::Clamp(Context.TargetHealth, 0.f, 1.f);
		break;
	case ENeuroFuzzyTacticalInput::GroupSize:
		Value = (float)Context.GroupSize;
		break;
	case ENeuroFuzzyTacticalInput::TargetGroupSize:
		Value = (float)Context.TargetGroupSize;
		break;
	case ENeuroFuzzyTacticalInput::CombatPressure:
		Value = GetPressureValue(Context);
		break;
	case ENeuroFuzzyTacticalInput::GroupSpread:
		Value = Context.AverageDistanceBetweenMembers;
		break;
	case ENeuroFuzzyTacticalInput::TargetMovement:
		Value = Context.TargetVelocity;
		break;
	default:
		break;
	}

	switch (Input)
	{
	case ENeuroFuzzyTacticalInput::DistanceToTarget:
		if (Set == ENeuroFuzzyMembershipSet::Low) return RampDown(Value, 250.f, 800.f);
		if (Set == ENeuroFuzzyMembershipSet::Medium) return Triangle(Value, 300.f, 650.f, 1000.f);
		if (Set == ENeuroFuzzyMembershipSet::High) return RampUp(Value, 600.f, 1200.f);
		break;
	case ENeuroFuzzyTacticalInput::AverageGroupHealth:
	case ENeuroFuzzyTacticalInput::LowestMemberHealth:
	case ENeuroFuzzyTacticalInput::TargetHealth:
		if (Set == ENeuroFuzzyMembershipSet::Low) return RampDown(Value, 0.25f, 0.6f);
		if (Set == ENeuroFuzzyMembershipSet::Medium) return Triangle(Value, 0.3f, 0.55f, 0.8f);
		if (Set == ENeuroFuzzyMembershipSet::High) return RampUp(Value, 0.5f, 0.9f);
		break;
	case ENeuroFuzzyTacticalInput::GroupSize:
	case ENeuroFuzzyTacticalInput::TargetGroupSize:
		if (Set == ENeuroFuzzyMembershipSet::Low) return RampDown(Value, 2.f, 4.f);
		if (Set == ENeuroFuzzyMembershipSet::Medium) return Triangle(Value, 3.f, 5.f, 7.f);
		if (Set == ENeuroFuzzyMembershipSet::High) return RampUp(Value, 5.f, 8.f);
		break;
	case ENeuroFuzzyTacticalInput::CombatPressure:
		if (Set == ENeuroFuzzyMembershipSet::Low) return RampDown(Value, 0.2f, 0.5f);
		if (Set == ENeuroFuzzyMembershipSet::Medium) return Triangle(Value, 0.25f, 0.5f, 0.75f);
		if (Set == ENeuroFuzzyMembershipSet::High) return RampUp(Value, 0.45f, 0.85f);
		break;
	case ENeuroFuzzyTacticalInput::GroupSpread:
		if (Set == ENeuroFuzzyMembershipSet::Low) return RampDown(Value, 150.f, 450.f);
		if (Set == ENeuroFuzzyMembershipSet::Medium) return Triangle(Value, 200.f, 400.f, 650.f);
		if (Set == ENeuroFuzzyMembershipSet::High) return RampUp(Value, 450.f, 750.f);
		break;
	case ENeuroFuzzyTacticalInput::TargetMovement:
		if (Set == ENeuroFuzzyMembershipSet::Low) return RampDown(Value, 150.f, 500.f);
		if (Set == ENeuroFuzzyMembershipSet::Medium) return Triangle(Value, 250.f, 550.f, 850.f);
		if (Set == ENeuroFuzzyMembershipSet::High) return RampUp(Value, 600.f, 1000.f);
		break;
	default:
		break;
	}

	return 0.f;
}

float FEnemyNeuroFuzzyEvaluator::GetRuleFiringStrength(const FGroupCombatContext& Context, const FNeuroFuzzyRule& Rule)
{
	if (Rule.Conditions.Num() == 0)
	{
		return 0.f;
	}

	float Strength = 1.f;
	for (const FNeuroFuzzyCondition& Condition : Rule.Conditions)
	{
		Strength *= GetMembershipValue(Context, Condition.Input, Condition.MembershipSet);
		if (Strength <= KINDA_SMALL_NUMBER)
		{
			return 0.f;
		}
	}

	return FMath::Clamp(Strength, 0.f, 1.f);
}

float FEnemyNeuroFuzzyEvaluator::GetPressureValue(const FGroupCombatContext& Context)
{
	const float Sum = Context.DamageTakenRecently + Context.DamageDealtRecently;
	if (Sum <= KINDA_SMALL_NUMBER)
	{
		return Context.bTakingHeavyLosses ? 0.75f : 0.25f;
	}

	return FMath::Clamp(Context.DamageTakenRecently / Sum, 0.f, 1.f);
}

float FEnemyNeuroFuzzyEvaluator::RampUp(float Value, float Min, float Max)
{
	return FMath::Clamp((Value - Min) / FMath::Max(KINDA_SMALL_NUMBER, Max - Min), 0.f, 1.f);
}

float FEnemyNeuroFuzzyEvaluator::RampDown(float Value, float Min, float Max)
{
	return 1.f - RampUp(Value, Min, Max);
}

float FEnemyNeuroFuzzyEvaluator::Triangle(float Value, float Min, float Peak, float Max)
{
	if (Value <= Min || Value >= Max)
	{
		return 0.f;
	}

	if (Value <= Peak)
	{
		return RampUp(Value, Min, Peak);
	}

	return RampDown(Value, Peak, Max);
}

void FEnemyNeuroFuzzyEvaluator::AccumulateOutputScore(
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
)
{
	if (bTargetsTuningPreset)
	{
		if (TuningOutput == EFormationTuningPreset::Compact)
		{
			Scores.CompactTuningScore += WeightedConsequent;
			CompactWeight += FiringStrength;
		}
		else if (TuningOutput == EFormationTuningPreset::Spread)
		{
			Scores.SpreadTuningScore += WeightedConsequent;
			SpreadWeight += FiringStrength;
		}
		else
		{
			Scores.BalancedTuningScore += WeightedConsequent;
			BalancedWeight += FiringStrength;
		}
		return;
	}

	if (FormationOutput == EFormationType::Circle)
	{
		Scores.CircleFormationScore += WeightedConsequent;
		CircleWeight += FiringStrength;
	}
	else if (FormationOutput == EFormationType::Line)
	{
		Scores.LineFormationScore += WeightedConsequent;
		LineWeight += FiringStrength;
	}
	else if (FormationOutput == EFormationType::Wedge)
	{
		Scores.WedgeFormationScore += WeightedConsequent;
		WedgeWeight += FiringStrength;
	}
}

void FEnemyNeuroFuzzyEvaluator::NormalizeOutputScores(
	FNeuroFuzzyOutputScores& Scores,
	float CircleWeight,
	float LineWeight,
	float WedgeWeight,
	float CompactWeight,
	float BalancedWeight,
	float SpreadWeight
)
{
	if (CircleWeight > KINDA_SMALL_NUMBER)
	{
		Scores.CircleFormationScore /= CircleWeight;
	}
	if (LineWeight > KINDA_SMALL_NUMBER)
	{
		Scores.LineFormationScore /= LineWeight;
	}
	if (WedgeWeight > KINDA_SMALL_NUMBER)
	{
		Scores.WedgeFormationScore /= WedgeWeight;
	}
	if (CompactWeight > KINDA_SMALL_NUMBER)
	{
		Scores.CompactTuningScore /= CompactWeight;
	}
	if (BalancedWeight > KINDA_SMALL_NUMBER)
	{
		Scores.BalancedTuningScore /= BalancedWeight;
	}
	if (SpreadWeight > KINDA_SMALL_NUMBER)
	{
		Scores.SpreadTuningScore /= SpreadWeight;
	}
}
