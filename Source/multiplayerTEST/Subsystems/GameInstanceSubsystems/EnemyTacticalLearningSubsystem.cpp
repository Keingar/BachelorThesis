#include "EnemyTacticalLearningSubsystem.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "multiplayerTEST/AI/EnemyGroupTacticalAI.h"
#include "multiplayerTEST/AI/EnemyNeuroFuzzyEvaluator.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/EnemyTacticalAISaveGame.h"

const FString UEnemyTacticalLearningSubsystem::SaveSlotName = TEXT("TacticalAIData");

namespace
{
	int32 GetGroupSizeBucket(int32 Size)
	{
		if (Size <= 2) return 0;
		if (Size <= 5) return 1;
		return 2;
	}

	int32 GetDistanceBucket(float Distance)
	{
		if (Distance <= 400.f) return 0;
		if (Distance <= 900.f) return 1;
		return 2;
	}

	float GetFormationSpreadFactor(float SpreadDistance)
	{
		const float Close = 50.f;
		const float Far = 500.f;
		const float Norm = (SpreadDistance - Close) / (Far - Close);
		const float T = FMath::Clamp(Norm, 0.f, 1.f);
		return T * T * (3.f - 2.f * T);
	}
}

void UEnemyTacticalLearningSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadLearningData();
}

void UEnemyTacticalLearningSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SaveTimer);
	}

	if (bSaveDirty)
	{
		SaveLearningData();
	}

	Super::Deinitialize();
}

float UEnemyTacticalLearningSubsystem::GetPreferenceWeight(EFormationType FormationType) const
{
	switch (FormationType)
	{
	case EFormationType::Circle:
		return CirclePreferenceWeight;
	case EFormationType::Line:
		return LinePreferenceWeight;
	case EFormationType::Wedge:
		return WedgePreferenceWeight;
	default:
		return 1.f;
	}
}

float UEnemyTacticalLearningSubsystem::GetContextBias(const FGroupCombatContext& Context, EFormationType FormationType) const
{
	const FFormationPerformanceData* Perf = ContextualPerformance.Find(MakeContextKey(Context, FormationType));
	if (!Perf || Perf->TimesUsed < 3)
	{
		return 0.f;
	}

	const float SuccessSignal = FMath::Clamp(Perf->SuccessScore / FMath::Max(1, Perf->TimesUsed), -1.f, 1.f);
	return FMath::Clamp(SuccessSignal * 15.f, -15.f, 15.f);
}

EFormationTuningPreset UEnemyTacticalLearningSubsystem::ChooseTuningPreset(const FGroupCombatContext& Context, EFormationType FormationType) const
{
	if (FormationType == EFormationType::None)
	{
		return EFormationTuningPreset::Balanced;
	}

	EFormationTuningPreset BestPreset = EFormationTuningPreset::Balanced;
	float BestScore = GetTuningPresetScore(Context, FormationType, EFormationTuningPreset::Balanced);

	const float CompactScore = GetTuningPresetScore(Context, FormationType, EFormationTuningPreset::Compact);
	if (CompactScore > BestScore)
	{
		BestPreset = EFormationTuningPreset::Compact;
		BestScore = CompactScore;
	}

	const float SpreadScore = GetTuningPresetScore(Context, FormationType, EFormationTuningPreset::Spread);
	if (SpreadScore > BestScore)
	{
		BestPreset = EFormationTuningPreset::Spread;
	}

	return BestPreset;
}

FNeuroFuzzyOutputScores UEnemyTacticalLearningSubsystem::EvaluateNeuroFuzzy(const FGroupCombatContext& Context) const
{
	return FEnemyNeuroFuzzyEvaluator::Evaluate(Context, NeuroFuzzyRuleConsequentOffsets);
}

void UEnemyTacticalLearningSubsystem::SubmitCombatResult(
	const FGroupCombatContext& Context,
	EFormationType UsedFormation,
	EFormationTuningPreset UsedTuningPreset,
	ECombatOutcome Outcome,
	float DamageDealt,
	float DamageTaken,
	float CombatDurationSeconds
)
{
	if (!ShouldProcessLearning() || UsedFormation == EFormationType::None)
	{
		return;
	}

	FTacticalContextKey Key = MakeContextKey(Context, UsedFormation);
	FFormationPerformanceData& Perf = ContextualPerformance.FindOrAdd(Key);
	UpdatePerformanceData(Perf, Outcome, DamageDealt, DamageTaken, CombatDurationSeconds);

	FTacticalTuningContextKey TuningKey = MakeTuningContextKey(Context, UsedFormation, UsedTuningPreset);
	FFormationPerformanceData& TuningPerf = TuningPerformance.FindOrAdd(TuningKey);
	UpdatePerformanceData(TuningPerf, Outcome, DamageDealt, DamageTaken, CombatDurationSeconds);

	const float TacticalSuccessScore = CalculateTacticalSuccessScore(
		Outcome,
		DamageDealt,
		DamageTaken,
		CombatDurationSeconds
	);

	if (float* Weight = GetMutablePreferenceWeight(UsedFormation))
	{
		AdjustWeightsFromPerformance(Perf, *Weight, TacticalSuccessScore);
	}

	UpdateNeuroFuzzyRuleLearning(Context, UsedFormation, UsedTuningPreset, TacticalSuccessScore);

	bSaveDirty = true;
	ScheduleSave();

	if (bEnableTacticalDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("TacticalLearning: Result G=%d D=%d TargetGroup=%d TG=%d Form=%d Preset=%d Success=%d Score=%.3f Weights C=%.2f L=%.2f W=%.2f Entries=%d TuningEntries=%d"),
			Key.GroupSizeBucket,
			Key.DistanceBucket,
			(int)Key.bTargetIsEnemyGroup,
			Key.TargetGroupSizeBucket,
			(int)Key.FormationType,
			(int)UsedTuningPreset,
			(int)(Outcome == ECombatOutcome::Victory),
			TacticalSuccessScore,
			CirclePreferenceWeight,
			LinePreferenceWeight,
			WedgePreferenceWeight,
			ContextualPerformance.Num(),
			TuningPerformance.Num());
	}
}

void UEnemyTacticalLearningSubsystem::SaveLearningData()
{
	if (!ShouldProcessLearning())
	{
		return;
	}

	UEnemyTacticalAISaveGame* SaveObj = Cast<UEnemyTacticalAISaveGame>(
		UGameplayStatics::CreateSaveGameObject(UEnemyTacticalAISaveGame::StaticClass())
	);
	if (!SaveObj)
	{
		return;
	}

	SaveObj->SaveVersion = 4;
	SaveObj->CirclePreferenceWeight = CirclePreferenceWeight;
	SaveObj->LinePreferenceWeight = LinePreferenceWeight;
	SaveObj->WedgePreferenceWeight = WedgePreferenceWeight;
	SaveObj->CircleMomentum = 0.f;
	SaveObj->LineMomentum = 0.f;
	SaveObj->ContextPerformanceMap = ContextualPerformance;
	SaveObj->TuningPerformanceMap = TuningPerformance;
	SaveObj->NeuroFuzzyRuleConsequentOffsets = NeuroFuzzyRuleConsequentOffsets;

	const bool bOk = UGameplayStatics::SaveGameToSlot(SaveObj, SaveSlotName, 0);
	if (bOk)
	{
		bSaveDirty = false;
	}

	bSavePending = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SaveTimer);
	}

	if (bEnableTacticalDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("TacticalLearning: SaveLearningData result=%d entries=%d tuningEntries=%d neuroFuzzyRules=%d"),
			(int)bOk,
			SaveObj->ContextPerformanceMap.Num(),
			SaveObj->TuningPerformanceMap.Num(),
			SaveObj->NeuroFuzzyRuleConsequentOffsets.Num());
	}
}

void UEnemyTacticalLearningSubsystem::LoadLearningData()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		if (bEnableTacticalDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("TacticalLearning: No save found, using defaults"));
		}
		return;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0);
	UEnemyTacticalAISaveGame* SG = Cast<UEnemyTacticalAISaveGame>(Loaded);
	if (!SG)
	{
		if (bEnableTacticalDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("TacticalLearning: Save slot incompatible"));
		}
		return;
	}

	CirclePreferenceWeight = FMath::Clamp(SG->CirclePreferenceWeight, 0.1f, 3.f);
	LinePreferenceWeight = FMath::Clamp(SG->LinePreferenceWeight, 0.1f, 3.f);
	WedgePreferenceWeight = FMath::Clamp(SG->WedgePreferenceWeight, 0.1f, 3.f);
	ContextualPerformance = SG->ContextPerformanceMap;
	TuningPerformance = SG->TuningPerformanceMap;
	NeuroFuzzyRuleConsequentOffsets = SG->NeuroFuzzyRuleConsequentOffsets;
	bSaveDirty = false;

	if (bEnableTacticalDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("TacticalLearning: Loaded save v%d CircleW=%.3f LineW=%.3f WedgeW=%.3f contexts=%d tuningContexts=%d neuroFuzzyRules=%d"),
			SG->SaveVersion,
			CirclePreferenceWeight,
			LinePreferenceWeight,
			WedgePreferenceWeight,
			ContextualPerformance.Num(),
			TuningPerformance.Num(),
			NeuroFuzzyRuleConsequentOffsets.Num());
	}
}

FTacticalContextKey UEnemyTacticalLearningSubsystem::MakeContextKey(const FGroupCombatContext& Context, EFormationType FormationType)
{
	FTacticalContextKey Key;
	Key.GroupSizeBucket = GetGroupSizeBucket(Context.GroupSize);
	Key.DistanceBucket = GetDistanceBucket(Context.DistanceToTarget);
	Key.bTargetIsEnemyGroup = Context.bTargetIsEnemyGroup;
	Key.TargetGroupSizeBucket = Context.bTargetIsEnemyGroup ? GetGroupSizeBucket(Context.TargetGroupSize) : 0;
	Key.FormationType = FormationType;
	return Key;
}

FTacticalTuningContextKey UEnemyTacticalLearningSubsystem::MakeTuningContextKey(const FGroupCombatContext& Context, EFormationType FormationType, EFormationTuningPreset TuningPreset)
{
	FTacticalTuningContextKey Key;
	Key.ContextKey = MakeContextKey(Context, FormationType);
	Key.TuningPreset = TuningPreset;
	return Key;
}

bool UEnemyTacticalLearningSubsystem::ShouldProcessLearning() const
{
	const UWorld* World = GetWorld();
	return !World || World->GetNetMode() != NM_Client;
}

void UEnemyTacticalLearningSubsystem::ScheduleSave(float DelaySeconds)
{
	if (bSavePending)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		SaveLearningData();
		return;
	}

	bSavePending = true;
	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &UEnemyTacticalLearningSubsystem::SaveLearningData);
	World->GetTimerManager().SetTimer(SaveTimer, Delegate, DelaySeconds, false);
}

float UEnemyTacticalLearningSubsystem::CalculateTacticalSuccessScore(
	ECombatOutcome Outcome,
	float DamageDealt,
	float DamageTaken,
	float CombatDurationSeconds
) const
{
	float DamageEfficiency = 0.f;
	const float SumDamage = DamageDealt + DamageTaken;
	if (SumDamage > KINDA_SMALL_NUMBER)
	{
		DamageEfficiency = DamageDealt / SumDamage;
	}

	float SurvivalFactor = 0.5f;
	if (Outcome == ECombatOutcome::Victory)
	{
		SurvivalFactor = 1.f;
	}
	else if (Outcome == ECombatOutcome::LeaderOnly)
	{
		SurvivalFactor = 0.75f;
	}

	const float DurationFactor = 1.f / (1.f + CombatDurationSeconds / 60.f);
	const float TacticalSuccessScore = (DamageEfficiency * 0.6f) + ((SurvivalFactor > 0.9f) ? 0.3f : 0.f) + (DurationFactor * 0.1f);
	return FMath::Clamp((TacticalSuccessScore - 0.5f) * 2.f, -1.f, 1.f);
}

void UEnemyTacticalLearningSubsystem::UpdatePerformanceData(
	FFormationPerformanceData& Perf,
	ECombatOutcome Outcome,
	float DamageDealt,
	float DamageTaken,
	float CombatDurationSeconds
)
{
	Perf.TimesUsed++;
	Perf.AverageDamageDealt = FMath::Lerp(Perf.AverageDamageDealt, DamageDealt, 1.f / Perf.TimesUsed);
	Perf.AverageDamageTaken = FMath::Lerp(Perf.AverageDamageTaken, DamageTaken, 1.f / Perf.TimesUsed);
	Perf.AverageCombatDuration = FMath::Lerp(Perf.AverageCombatDuration, CombatDurationSeconds, 1.f / Perf.TimesUsed);

	if (Outcome == ECombatOutcome::Victory)
	{
		Perf.Victories++;
		Perf.SuccessScore += 1.f;
	}
	else if (Outcome == ECombatOutcome::Defeat)
	{
		Perf.Failures++;
		Perf.SuccessScore -= 1.f;
	}
	else
	{
		Perf.Failures++;
		Perf.SuccessScore -= 0.5f;
	}
}

void UEnemyTacticalLearningSubsystem::AdjustWeightsFromPerformance(FFormationPerformanceData& Perf, float& PreferenceWeight, float TacticalSuccessScore)
{
	const float BaseLearn = 0.05f;
	const float Delta = FMath::Clamp(TacticalSuccessScore, -1.f, 1.f) * BaseLearn;

	float DamageFactor = 0.f;
	const float DamageSum = Perf.AverageDamageDealt + Perf.AverageDamageTaken;
	if (DamageSum > KINDA_SMALL_NUMBER)
	{
		DamageFactor = (Perf.AverageDamageDealt - Perf.AverageDamageTaken) / DamageSum;
	}

	PreferenceWeight = FMath::Clamp(PreferenceWeight + Delta + DamageFactor * 0.02f, 0.1f, 3.0f);
}

float* UEnemyTacticalLearningSubsystem::GetMutablePreferenceWeight(EFormationType FormationType)
{
	switch (FormationType)
	{
	case EFormationType::Circle:
		return &CirclePreferenceWeight;
	case EFormationType::Line:
		return &LinePreferenceWeight;
	case EFormationType::Wedge:
		return &WedgePreferenceWeight;
	default:
		return nullptr;
	}
}

float UEnemyTacticalLearningSubsystem::GetTuningPresetScore(
	const FGroupCombatContext& Context,
	EFormationType FormationType,
	EFormationTuningPreset TuningPreset
) const
{
	float Score = (TuningPreset == EFormationTuningPreset::Balanced) ? 2.f : 0.f;

	if (TuningPreset == EFormationTuningPreset::Compact)
	{
		Score += GetFormationSpreadFactor(Context.AverageDistanceBetweenMembers) * 4.f;
		Score += Context.bTargetInsideFormation ? 3.f : 0.f;
		Score += Context.bTakingHeavyLosses ? -3.f : 0.f;
	}
	else if (TuningPreset == EFormationTuningPreset::Spread)
	{
		Score += Context.bTakingHeavyLosses ? 4.f : 0.f;
		Score += Context.bTargetIsEnemyGroup ? 2.f : 0.f;
		Score += Context.bGroupSpreadTooFar ? -2.f : 0.f;
	}

	if (FormationType == EFormationType::Wedge && TuningPreset == EFormationTuningPreset::Spread)
	{
		Score += Context.TargetGroupSize >= 6 ? 2.f : 0.f;
	}

	Score += GetNeuroFuzzyTuningPresetScore(Context, FormationType, TuningPreset) * NeuroFuzzyTuningScoreScale;

	const FFormationPerformanceData* Perf = TuningPerformance.Find(MakeTuningContextKey(Context, FormationType, TuningPreset));
	if (Perf && Perf->TimesUsed >= 2)
	{
		const float SuccessSignal = FMath::Clamp(Perf->SuccessScore / FMath::Max(1, Perf->TimesUsed), -1.f, 1.f);
		Score += FMath::Clamp(SuccessSignal * 10.f, -10.f, 10.f);
	}

	return Score;
}

float UEnemyTacticalLearningSubsystem::GetNeuroFuzzyTuningPresetScore(
	const FGroupCombatContext& Context,
	EFormationType FormationType,
	EFormationTuningPreset TuningPreset
) const
{
	float WeightedScore = 0.f;
	float WeightSum = 0.f;

	for (const FNeuroFuzzyRuleActivation& Activation : FEnemyNeuroFuzzyEvaluator::EvaluateRuleActivations(Context))
	{
		const FNeuroFuzzyRule& Rule = Activation.Rule;
		if (!Rule.bTargetsTuningPreset || Rule.TuningOutput != TuningPreset)
		{
			continue;
		}

		if (Rule.FormationOutput != EFormationType::None && Rule.FormationOutput != FormationType)
		{
			continue;
		}

		if (Activation.FiringStrength <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		const float LearnedOffset = NeuroFuzzyRuleConsequentOffsets.FindRef(Rule.RuleId);
		const float Consequent = FMath::Clamp(Rule.Consequent + LearnedOffset, 0.f, 1.f);
		WeightedScore += Activation.FiringStrength * Consequent;
		WeightSum += Activation.FiringStrength;
	}

	return WeightSum > KINDA_SMALL_NUMBER ? WeightedScore / WeightSum : 0.f;
}

void UEnemyTacticalLearningSubsystem::UpdateNeuroFuzzyRuleLearning(
	const FGroupCombatContext& Context,
	EFormationType UsedFormation,
	EFormationTuningPreset UsedTuningPreset,
	float TacticalSuccessScore
)
{
	constexpr float LearningRate = 0.04f;
	constexpr float MinimumActiveStrength = 0.05f;

	for (const FNeuroFuzzyRuleActivation& Activation : FEnemyNeuroFuzzyEvaluator::EvaluateRuleActivations(Context))
	{
		const FNeuroFuzzyRule& Rule = Activation.Rule;
		if (Rule.RuleId.IsNone() || Activation.FiringStrength < MinimumActiveStrength)
		{
			continue;
		}

		const bool bMatchesUsedFormationRule =
			!Rule.bTargetsTuningPreset &&
			Rule.FormationOutput == UsedFormation;

		const bool bMatchesUsedTuningRule =
			Rule.bTargetsTuningPreset &&
			(Rule.FormationOutput == UsedFormation || Rule.FormationOutput == EFormationType::None) &&
			Rule.TuningOutput == UsedTuningPreset;

		if (!bMatchesUsedFormationRule && !bMatchesUsedTuningRule)
		{
			continue;
		}

		float& LearnedOffset = NeuroFuzzyRuleConsequentOffsets.FindOrAdd(Rule.RuleId);
		const float Delta = TacticalSuccessScore * Activation.FiringStrength * LearningRate;
		LearnedOffset = FMath::Clamp(LearnedOffset + Delta, -0.35f, 0.35f);
	}
}
