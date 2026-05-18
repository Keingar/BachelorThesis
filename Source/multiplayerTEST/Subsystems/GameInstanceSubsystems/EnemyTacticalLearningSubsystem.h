#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/FormationDecision.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/FormationPerformance.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/GroupCombatContext.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/NeuroFuzzyTacticalModel.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/TacticalContextKey.h"
#include "EnemyTacticalLearningSubsystem.generated.h"

enum class ECombatOutcome : uint8;

UCLASS()
class MULTIPLAYERTEST_API UEnemyTacticalLearningSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	float GetPreferenceWeight(EFormationType FormationType) const;
	float GetContextBias(const FGroupCombatContext& Context, EFormationType FormationType) const;
	EFormationTuningPreset ChooseTuningPreset(const FGroupCombatContext& Context, EFormationType FormationType) const;
	FNeuroFuzzyOutputScores EvaluateNeuroFuzzy(const FGroupCombatContext& Context) const;

	void SubmitCombatResult(
		const FGroupCombatContext& Context,
		EFormationType UsedFormation,
		EFormationTuningPreset UsedTuningPreset,
		ECombatOutcome Outcome,
		float DamageDealt,
		float DamageTaken,
		float CombatDurationSeconds
	);

	void SaveLearningData();
	void LoadLearningData();

	static FTacticalContextKey MakeContextKey(const FGroupCombatContext& Context, EFormationType FormationType);
	static FTacticalTuningContextKey MakeTuningContextKey(const FGroupCombatContext& Context, EFormationType FormationType, EFormationTuningPreset TuningPreset);

private:
	static const FString SaveSlotName;

	UPROPERTY()
	float CirclePreferenceWeight = 1.f;

	UPROPERTY()
	float LinePreferenceWeight = 1.f;

	UPROPERTY()
	float WedgePreferenceWeight = 1.f;

	UPROPERTY()
	TMap<FTacticalContextKey, FFormationPerformanceData> ContextualPerformance;

	UPROPERTY()
	TMap<FTacticalTuningContextKey, FFormationPerformanceData> TuningPerformance;

	UPROPERTY()
	TMap<FName, float> NeuroFuzzyRuleConsequentOffsets;

	UPROPERTY(EditAnywhere)
	bool bEnableTacticalDebug = true;

	UPROPERTY(EditAnywhere)
	float NeuroFuzzyTuningScoreScale = 6.f;

	FTimerHandle SaveTimer;
	bool bSaveDirty = false;
	bool bSavePending = false;

	bool ShouldProcessLearning() const;
	void ScheduleSave(float DelaySeconds = 3.0f);
	float CalculateTacticalSuccessScore(ECombatOutcome Outcome, float DamageDealt, float DamageTaken, float CombatDurationSeconds) const;
	void UpdatePerformanceData(FFormationPerformanceData& Perf, ECombatOutcome Outcome, float DamageDealt, float DamageTaken, float CombatDurationSeconds);
	void AdjustWeightsFromPerformance(FFormationPerformanceData& Perf, float& PreferenceWeight, float TacticalSuccessScore);
	float* GetMutablePreferenceWeight(EFormationType FormationType);
	float GetTuningPresetScore(const FGroupCombatContext& Context, EFormationType FormationType, EFormationTuningPreset TuningPreset) const;
	float GetNeuroFuzzyTuningPresetScore(const FGroupCombatContext& Context, EFormationType FormationType, EFormationTuningPreset TuningPreset) const;
	void UpdateNeuroFuzzyRuleLearning(
		const FGroupCombatContext& Context,
		EFormationType UsedFormation,
		EFormationTuningPreset UsedTuningPreset,
		float TacticalSuccessScore
	);
};
