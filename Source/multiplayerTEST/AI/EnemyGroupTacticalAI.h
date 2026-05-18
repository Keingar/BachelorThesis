// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/FormationDecision.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/GroupCombatContext.h"

#include "EnemyGroupTacticalAI.generated.h"

class UEnemyTacticalLearningSubsystem;

UENUM(BlueprintType)
enum class ETacticalState : uint8
{
	Aggressive UMETA(DisplayName = "Aggressive"),
	Defensive UMETA(DisplayName = "Defensive"),
	Surround   UMETA(DisplayName = "Surround"),
	Retreat    UMETA(DisplayName = "Retreat")
};

UENUM(BlueprintType)
enum class ECombatOutcome : uint8
{
	Victory    UMETA(DisplayName = "Victory"),
	Defeat     UMETA(DisplayName = "Defeat"),
	LeaderOnly UMETA(DisplayName = "LeaderOnly")
};

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UEnemyGroupTacticalAI : public UObject
{
	GENERATED_BODY()
	
public:
    FFormationDecision CalculateDecision(FGroupCombatContext Context);

    UEnemyGroupTacticalAI(const FObjectInitializer& ObjectInitializer);

	// Persistent learning API
	UFUNCTION()
	void SaveLearningData();

	UFUNCTION()
	void LoadLearningData();

	// Recent bias (simple momentum accumulator)
	UPROPERTY()
	float RecentSuccessBias = 0.f;

	// Adaptive learning: expose notification when combat ends so tactical AI
	// can update formation performance and adjust preference weights.
    UFUNCTION()
	void NotifyCombatEnded(EFormationType UsedFormation, EFormationTuningPreset UsedTuningPreset, ECombatOutcome Outcome, float DamageDealt, float DamageTaken, float CombatDurationSeconds);

	// Expose current preference weights for debugging / tuning
	UPROPERTY(BlueprintReadWrite)
	float CirclePreferenceWeight = 1.f;

	UPROPERTY(BlueprintReadWrite)
	float LinePreferenceWeight = 1.f;

	UPROPERTY(BlueprintReadWrite)
	float WedgePreferenceWeight = 1.f;

protected:
	// Utility scoring for formations
	float CalculateCircleScore(const FGroupCombatContext& Context) const;
	float CalculateLineScore(const FGroupCombatContext& Context) const;
	float CalculateWedgeScore(const FGroupCombatContext& Context) const;

	// Fuzzy helper functions (smooth 0..1 outputs)
	float GetCloseRangeFactor(float Distance) const;
    float GetMediumRangeFactor(float Distance) const;
	float GetFarRangeFactor(float Distance) const;
    // Danger/health
	float GetHealthFactor(float NormalizedHealth) const;
	float GetDangerFactor(float HealthPercent) const;

	// Pressure: overloads for different sources
	float GetPressureFactor(float DamageTaken, float DamageDealt) const;
	float GetPressureFactor(int32 GroupSize) const;

	// Spread/formation
	float GetSpreadFactor(float AvgDistanceBetweenMembers) const;
	float GetFormationSpreadFactor(float SpreadDistance) const;
	float GetTargetMovementFactor(float TargetVelocity) const;

protected:
	// Last observed context (set during CalculateDecision) used by NotifyCombatEnded
	UPROPERTY()
	FGroupCombatContext LastObservedContext;

	// Formation change cooldown management
	UPROPERTY()
	double LastFormationChangeTime = 0.0;

	UPROPERTY(EditAnywhere)
	float FormationChangeCooldown = 10.f;

	// Minimum utility gain required to justify changing formation. If the
	// best formation's utility minus the alternative is less than this value,
	// the AI will prefer to keep the current formation. Tunable in editor.
	UPROPERTY(EditAnywhere)
	float FormationChangeUtilityThreshold = 15.f;

    // Tactical state
	UPROPERTY(BlueprintReadOnly)
	ETacticalState CurrentTacticalState = ETacticalState::Aggressive;

	// Momentum per formation
	UPROPERTY()
	float CircleMomentum = 0.f;

	UPROPERTY()
	float LineMomentum = 0.f;

	UPROPERTY()
	float WedgeMomentum = 0.f;

	UPROPERTY(EditAnywhere)
	bool bEnableTacticalDebug = true;

	UPROPERTY(EditAnywhere, Category = "Tactical AI|Neuro Fuzzy")
	bool bEnableNeuroFuzzyTacticalLayer = true;

	UPROPERTY(EditAnywhere, Category = "Tactical AI|Neuro Fuzzy")
	float NeuroFuzzyFormationScoreScale = 12.f;

	UPROPERTY(EditAnywhere, Category = "Tactical AI|Variation")
	bool bEnableTacticalDecisionVariation = true;

	UPROPERTY(EditAnywhere, Category = "Tactical AI|Variation", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "10.0"))
	float FormationScoreVariationRange = 4.f;

	UPROPERTY(EditAnywhere, Category = "Tactical AI|Variation", meta = (ClampMin = "0.0", ClampMax = "0.25", UIMin = "0.0", UIMax = "0.15"))
	float FormationParameterVariationPercent = 0.06f;

	// Last chosen formation and time
	UPROPERTY()
	EFormationType LastChosenFormation = EFormationType::None;

	UEnemyTacticalLearningSubsystem* GetLearningSubsystem() const;
	void ApplyParameterVariation(FFormationDecision& Decision) const;
	float CalculateTacticalSuccessScore(ECombatOutcome Outcome, float DamageDealt, float DamageTaken, float CombatDurationSeconds) const;
};
