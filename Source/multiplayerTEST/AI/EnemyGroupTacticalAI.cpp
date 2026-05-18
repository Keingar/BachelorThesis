// Tactical AI implementation using utility scoring
#include "EnemyGroupTacticalAI.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "Math/UnrealMathUtility.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/EnemyTacticalLearningSubsystem.h"

UEnemyGroupTacticalAI::UEnemyGroupTacticalAI(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

static float SmoothStep(float t)
{
    t = FMath::Clamp(t, 0.f, 1.f);
    return t * t * (3.f - 2.f * t);
}

static void ApplyTuningPreset(FFormationDecision& Decision)
{
    if (Decision.TuningPreset == EFormationTuningPreset::Balanced)
    {
        return;
    }

    const bool bCompact = Decision.TuningPreset == EFormationTuningPreset::Compact;
    const float TightFactor = bCompact ? 0.82f : 1.20f;
    const float DepthFactor = bCompact ? 0.80f : 1.25f;
    const float DistanceFactor = bCompact ? 0.90f : 1.10f;

    if (Decision.FormationType == EFormationType::Circle)
    {
        Decision.CircleRadius = FMath::Clamp(Decision.CircleRadius * (bCompact ? 0.82f : 1.18f), 150.f, 1400.f);
        Decision.CircleLayerRadiusOffset = FMath::Clamp(Decision.CircleLayerRadiusOffset * DepthFactor, 80.f, 420.f);
        Decision.CircleMinAnglePerMemberDeg = FMath::Clamp(Decision.CircleMinAnglePerMemberDeg * (bCompact ? 0.92f : 1.10f), 18.f, 80.f);
    }
    else if (Decision.FormationType == EFormationType::Line)
    {
        Decision.LineMinSpacing = FMath::Clamp(Decision.LineMinSpacing * TightFactor, 90.f, 260.f);
        Decision.LineMaxLineLength = FMath::Clamp(Decision.LineMaxLineLength * (bCompact ? 0.85f : 1.25f), 300.f, 20000.f);
        Decision.LineRowOffset = FMath::Clamp(Decision.LineRowOffset * DepthFactor, 100.f, 420.f);
    }
    else if (Decision.FormationType == EFormationType::Wedge)
    {
        Decision.WedgeDesiredDistanceToTarget = FMath::Clamp(Decision.WedgeDesiredDistanceToTarget * DistanceFactor, 500.f, 1000.f);
        Decision.WedgeSideSpacing = FMath::Clamp(Decision.WedgeSideSpacing * DepthFactor, 110.f, 320.f);
        Decision.WedgeDepthSpacing = FMath::Clamp(Decision.WedgeDepthSpacing * DepthFactor, 140.f, 420.f);
        Decision.WedgeLeaderOffset = FMath::Clamp(Decision.WedgeLeaderOffset * DistanceFactor, 220.f, 420.f);
    }
}

FFormationDecision UEnemyGroupTacticalAI::CalculateDecision(FGroupCombatContext Context)
{
    FFormationDecision Decision;
    // store last observed context for post-combat learning
    LastObservedContext = Context;

    // determine tactical state from context
    // simple rules: retreat if very low health, surround if large & close, defensive if low health or high pressure
    if (Context.AverageGroupHealth < 0.25f)
    {
        CurrentTacticalState = ETacticalState::Retreat;
    }
    else if (Context.AverageGroupHealth < 0.5f || Context.bTakingHeavyLosses)
    {
        CurrentTacticalState = ETacticalState::Defensive;
    }
    else if (Context.GroupSize >= 6 && GetCloseRangeFactor(Context.DistanceToTarget) > 0.5f)
    {
        CurrentTacticalState = ETacticalState::Surround;
    }
    else
    {
        CurrentTacticalState = ETacticalState::Aggressive;
    }

    UEnemyTacticalLearningSubsystem* LearningSubsystem = GetLearningSubsystem();
    if (LearningSubsystem)
    {
        CirclePreferenceWeight = LearningSubsystem->GetPreferenceWeight(EFormationType::Circle);
        LinePreferenceWeight = LearningSubsystem->GetPreferenceWeight(EFormationType::Line);
        WedgePreferenceWeight = LearningSubsystem->GetPreferenceWeight(EFormationType::Wedge);
    }

    // Decay momentum
    CircleMomentum *= 0.95f;
    LineMomentum *= 0.95f;
    WedgeMomentum *= 0.95f;

    // Compute base utility scores
    float CircleScore = CalculateCircleScore(Context) * CirclePreferenceWeight + CircleMomentum * 5.f;
    float LineScore = CalculateLineScore(Context) * LinePreferenceWeight + LineMomentum * 5.f;
    const bool bCanUseWedge = Context.bTargetIsEnemyGroup && Context.TargetGroupSize >= 3;
    float WedgeScore = bCanUseWedge ? CalculateWedgeScore(Context) * WedgePreferenceWeight + WedgeMomentum * 5.f : -TNumericLimits<float>::Max();

    if (LearningSubsystem)
    {
        CircleScore += LearningSubsystem->GetContextBias(Context, EFormationType::Circle);
        LineScore += LearningSubsystem->GetContextBias(Context, EFormationType::Line);
        if (bCanUseWedge)
        {
            WedgeScore += LearningSubsystem->GetContextBias(Context, EFormationType::Wedge);
        }
    }

    FNeuroFuzzyOutputScores NeuroFuzzyScores;
    if (bEnableNeuroFuzzyTacticalLayer && LearningSubsystem)
    {
        NeuroFuzzyScores = LearningSubsystem->EvaluateNeuroFuzzy(Context);
        CircleScore += NeuroFuzzyScores.CircleFormationScore * NeuroFuzzyFormationScoreScale;
        LineScore += NeuroFuzzyScores.LineFormationScore * NeuroFuzzyFormationScoreScale;
        if (bCanUseWedge)
        {
            WedgeScore += NeuroFuzzyScores.WedgeFormationScore * NeuroFuzzyFormationScoreScale;
        }
    }

    // Tactical state influence (soft bias)
    switch (CurrentTacticalState)
    {
    case ETacticalState::Aggressive:
        LineScore += 10.f;
        if (bCanUseWedge) WedgeScore += 10.f;
        break;
    case ETacticalState::Defensive:
        CircleScore += 15.f;
        if (bCanUseWedge) WedgeScore += 5.f;
        break;
    case ETacticalState::Surround:
        CircleScore += 25.f;
        if (bCanUseWedge) WedgeScore += 10.f;
        break;
    case ETacticalState::Retreat:
        CircleScore += 40.f;
        break;
    }

    // Formation change cooldown: if we changed recently, bias toward last chosen formation
    double Now = FPlatformTime::Seconds();
    const bool bLastChosenScoredFormation =
        LastChosenFormation == EFormationType::Circle ||
        LastChosenFormation == EFormationType::Line ||
        (LastChosenFormation == EFormationType::Wedge && bCanUseWedge);
    bool bCooldownActive = bLastChosenScoredFormation && (Now - LastFormationChangeTime) < FormationChangeCooldown;
    if (bCooldownActive && LastChosenFormation != EFormationType::None)
    {
        if (LastChosenFormation == EFormationType::Circle) CircleScore += 20.f;
        if (LastChosenFormation == EFormationType::Line) LineScore += 20.f;
        if (LastChosenFormation == EFormationType::Wedge && bCanUseWedge) WedgeScore += 20.f;
    }

    const float ScoreVariationRange = FMath::Max(0.f, FormationScoreVariationRange);
    if (bEnableTacticalDecisionVariation && ScoreVariationRange > KINDA_SMALL_NUMBER)
    {
        CircleScore += FMath::FRandRange(-ScoreVariationRange, ScoreVariationRange);
        LineScore += FMath::FRandRange(-ScoreVariationRange, ScoreVariationRange);
        if (bCanUseWedge)
        {
            WedgeScore += FMath::FRandRange(-ScoreVariationRange, ScoreVariationRange);
        }
    }

    // Selection with hysteresis: pick formation with higher utility
    // but only switch if the utility gain exceeds the configured threshold AND
    // the formation change cooldown has passed. This prevents frequent flipping.
    EFormationType Best = EFormationType::Circle;
    float BestScore = CircleScore;
    if (LineScore > BestScore)
    {
        Best = EFormationType::Line;
        BestScore = LineScore;
    }
    if (bCanUseWedge && WedgeScore > BestScore)
    {
        Best = EFormationType::Wedge;
        BestScore = WedgeScore;
    }

    auto GetScoreForFormation = [&](EFormationType FormationType) -> float
    {
        if (FormationType == EFormationType::Circle) return CircleScore;
        if (FormationType == EFormationType::Line) return LineScore;
        if (FormationType == EFormationType::Wedge && bCanUseWedge) return WedgeScore;
        return -TNumericLimits<float>::Max();
    };

    EFormationType ChosenFormation = Best;
    if (!bLastChosenScoredFormation)
    {
        ChosenFormation = Best;
    }
    else if (Best == LastChosenFormation)
    {
        ChosenFormation = Best;
    }
    else
    {
        const float CurrentScore = GetScoreForFormation(LastChosenFormation);
        const float UtilityDiff = BestScore - CurrentScore;
        if (!bCooldownActive && UtilityDiff >= FormationChangeUtilityThreshold)
        {
            ChosenFormation = Best;
        }
        else
        {
            ChosenFormation = LastChosenFormation;
        }
    }

    // Debug
    if (bEnableTacticalDebug)
    {
        UE_LOG(LogTemp, Warning, TEXT("TacticalAI: Decision GSize=%d Dist=%.1f TargetGroup=%d TargetSize=%d State=%d Circle=%.2f Line=%.2f Wedge=%.2f NFC=%.2f NFL=%.2f NFW=%.2f Chosen=%d Cooldown=%d"),
            Context.GroupSize,
            Context.DistanceToTarget,
            (int)Context.bTargetIsEnemyGroup,
            Context.TargetGroupSize,
            (int)CurrentTacticalState,
            CircleScore,
            LineScore,
            WedgeScore,
            NeuroFuzzyScores.CircleFormationScore,
            NeuroFuzzyScores.LineFormationScore,
            NeuroFuzzyScores.WedgeFormationScore,
            (int)ChosenFormation,
            (int)bCooldownActive);
    }

    // Build the decision object and record last chosen formation/time
    if (ChosenFormation == EFormationType::Wedge)
    {
        Decision.FormationType = EFormationType::Wedge;
        Decision.bSetWedgeParams = true;
        Decision.WedgeDesiredDistanceToTarget = 700.f;
        Decision.WedgeSideSpacing = FMath::Lerp(140.f, 220.f, GetSpreadFactor(Context.AverageDistanceBetweenMembers));
        Decision.WedgeDepthSpacing = FMath::Lerp(180.f, 280.f, GetPressureFactor(Context.DamageTakenRecently, Context.DamageDealtRecently));
        Decision.WedgeLeaderOffset = 300.f;
        Decision.bEnableBacklineSwap = (Context.LowestMemberHealth < 0.85f);
        Decision.BacklineHpDiffThresholdPercent = 40.f;
    }
    else if (ChosenFormation == EFormationType::Circle)
    {
        Decision.FormationType = EFormationType::Circle;
        Decision.bSetCircleParams = true;
        float baseRadius = 200.f + Context.GroupSize * 40.f;
        float radiusFromDist = Context.DistanceToTarget * 0.4f;
        Decision.CircleRadius = FMath::Clamp(FMath::Max(baseRadius, radiusFromDist), 200.f, 1200.f);
        Decision.CircleArcAngleDegrees = 360.f;
        Decision.CircleArcOffsetDegrees = 0.f;
        Decision.CircleMinAnglePerMemberDeg = FMath::Clamp(360.f / FMath::Max(1, Context.GroupSize), 20.f, 60.f);
        Decision.CircleLayerRadiusOffset = FMath::Lerp(120.f, 300.f, GetPressureFactor(Context.DamageTakenRecently, Context.DamageDealtRecently));
    }
    else
    {
        Decision.FormationType = EFormationType::Line;
        Decision.bSetLineParams = true;
        Decision.LineMinSpacing = FMath::Lerp(120.f, 200.f, GetSpreadFactor(Context.AverageDistanceBetweenMembers));
        Decision.LineMaxLineLength = FMath::Max(10000.f, Context.GroupSize * Decision.LineMinSpacing);
        Decision.LineRowOffset = FMath::Lerp(150.f, 300.f, GetPressureFactor(Context.DamageTakenRecently, Context.DamageDealtRecently));
    }

    Decision.TuningPreset = LearningSubsystem
        ? LearningSubsystem->ChooseTuningPreset(Context, Decision.FormationType)
        : EFormationTuningPreset::Balanced;
    ApplyTuningPreset(Decision);
    ApplyParameterVariation(Decision);

    // Decide whether to enable backline swapping: if some members are low on health,
    // allow swapping so weaker front-line members can move to backline.
    // Use a more sensitive threshold so swaps trigger earlier.
    Decision.bEnableBacklineSwap = (Context.LowestMemberHealth < 0.85f);
    Decision.BacklineHpDiffThresholdPercent = 40.f; // default threshold

    // record last chosen formation/time (only update change time if formation actually changed)
    if (Decision.FormationType != LastChosenFormation)
    {
        LastChosenFormation = Decision.FormationType;
        LastFormationChangeTime = Now;
    }

    return Decision;
}

void UEnemyGroupTacticalAI::ApplyParameterVariation(FFormationDecision& Decision) const
{
    if (!bEnableTacticalDecisionVariation)
    {
        return;
    }

    const float VariationPercent = FMath::Clamp(FormationParameterVariationPercent, 0.f, 0.25f);
    if (VariationPercent <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    auto RandomScale = [VariationPercent]() -> float
    {
        return 1.f + FMath::FRandRange(-VariationPercent, VariationPercent);
    };

    if (Decision.FormationType == EFormationType::Circle)
    {
        Decision.CircleRadius = FMath::Clamp(Decision.CircleRadius * RandomScale(), 150.f, 1400.f);
        Decision.CircleLayerRadiusOffset = FMath::Clamp(Decision.CircleLayerRadiusOffset * RandomScale(), 80.f, 420.f);
        Decision.CircleMinAnglePerMemberDeg = FMath::Clamp(Decision.CircleMinAnglePerMemberDeg * RandomScale(), 18.f, 80.f);
    }
    else if (Decision.FormationType == EFormationType::Line)
    {
        Decision.LineMinSpacing = FMath::Clamp(Decision.LineMinSpacing * RandomScale(), 90.f, 260.f);
        Decision.LineMaxLineLength = FMath::Clamp(Decision.LineMaxLineLength * RandomScale(), 300.f, 20000.f);
        Decision.LineRowOffset = FMath::Clamp(Decision.LineRowOffset * RandomScale(), 100.f, 420.f);
    }
    else if (Decision.FormationType == EFormationType::Wedge)
    {
        Decision.WedgeDesiredDistanceToTarget = FMath::Clamp(Decision.WedgeDesiredDistanceToTarget * RandomScale(), 500.f, 1000.f);
        Decision.WedgeSideSpacing = FMath::Clamp(Decision.WedgeSideSpacing * RandomScale(), 110.f, 320.f);
        Decision.WedgeDepthSpacing = FMath::Clamp(Decision.WedgeDepthSpacing * RandomScale(), 140.f, 420.f);
        Decision.WedgeLeaderOffset = FMath::Clamp(Decision.WedgeLeaderOffset * RandomScale(), 220.f, 420.f);
    }
}

float UEnemyGroupTacticalAI::CalculateCircleScore(const FGroupCombatContext& Context) const
{
    float Score = 0.f;

    // Size: larger groups favor circle (surrounding)
    Score += Context.GroupSize * 5.f;

    // Close range favor circle (defensive / surround)
    Score += GetCloseRangeFactor(Context.DistanceToTarget) * 40.f;

    // Medium range slightly favors circle for flexible positioning
    Score += GetMediumRangeFactor(Context.DistanceToTarget) * 10.f;

    // If group health is low, prefer circle (defensive)
    Score += GetDangerFactor(Context.AverageGroupHealth) * 40.f;

    // If taking pressure prefer circle
    Score += GetPressureFactor(Context.DamageTakenRecently, Context.DamageDealtRecently) * 30.f;
    // Also small groups may avoid circle - use group-size pressure factor
    Score += GetPressureFactor(Context.GroupSize) * 5.f;

    // Target movement: fast-moving targets reduce circle utility
    Score -= GetTargetMovementFactor(Context.TargetVelocity) * 20.f;

    // Spread: if group is spread out, circle helps regroup
    Score += GetSpreadFactor(Context.AverageDistanceBetweenMembers) * 20.f;

    return Score;
}

float UEnemyGroupTacticalAI::CalculateLineScore(const FGroupCombatContext& Context) const
{
    float Score = 0.f;

    // Size: medium groups favor line formation for hitting power
    Score += Context.GroupSize * 4.f;

    // Far range favors line (engage / chase)
    Score += GetFarRangeFactor(Context.DistanceToTarget) * 50.f;
    // Medium range can also suit line depending on spread
    Score += GetMediumRangeFactor(Context.DistanceToTarget) * 10.f;

    // Healthy groups prefer line (aggressive)
    Score += GetHealthFactor(Context.AverageGroupHealth) * 25.f;

    // Target moving fast favors line to chase
    Score += GetTargetMovementFactor(Context.TargetVelocity) * 30.f;

    // If under little pressure (not taking heavy losses), line is preferred
    Score += (1.f - GetPressureFactor(Context.DamageTakenRecently, Context.DamageDealtRecently)) * 20.f;
    // Larger groups increase line effectiveness
    Score += GetPressureFactor(Context.GroupSize) * 10.f;

    // Spread penalizes line
    Score -= GetFormationSpreadFactor(Context.AverageDistanceBetweenMembers) * 20.f;

    return Score;
}

float UEnemyGroupTacticalAI::CalculateWedgeScore(const FGroupCombatContext& Context) const
{
    if (!Context.bTargetIsEnemyGroup || Context.TargetGroupSize < 3)
    {
        return -100000.f;
    }

    float Score = 45.f;

    // Wedge is an aggressive anti-group shape: strongest when the group is healthy,
    // coordinated, and closing on a real enemy formation.
    Score += GetHealthFactor(Context.AverageGroupHealth) * 25.f;
    Score -= GetDangerFactor(Context.AverageGroupHealth) * 20.f;
    Score += GetMediumRangeFactor(Context.DistanceToTarget) * 20.f;
    Score += GetFarRangeFactor(Context.DistanceToTarget) * 15.f;
    Score += FMath::Clamp((float)Context.TargetGroupSize / 8.f, 0.f, 1.f) * 25.f;
    Score += (1.f - GetFormationSpreadFactor(Context.AverageDistanceBetweenMembers)) * 15.f;
    Score += (1.f - GetPressureFactor(Context.DamageTakenRecently, Context.DamageDealtRecently)) * 15.f;

    if (Context.bTargetEscaping)
    {
        Score += 10.f;
    }

    return Score;
}

// --- Fuzzy helper implementations ---
float UEnemyGroupTacticalAI::GetCloseRangeFactor(float Distance) const
{
    // 1 at <= 200, 0 at >= 800
    const float Near = 200.f;
    const float Far = 800.f;
    float norm = (Distance - Near) / (Far - Near);
    return 1.f - SmoothStep(norm);
}

float UEnemyGroupTacticalAI::GetMediumRangeFactor(float Distance) const
{
    // Peak medium roughly around 500. Rises from 300..600, falls 600..1000
    float rise = SmoothStep((Distance - 300.f) / (300.f));
    float fall = 1.f - SmoothStep((Distance - 600.f) / (400.f));
    return FMath::Clamp(rise * fall, 0.f, 1.f);
}

float UEnemyGroupTacticalAI::GetFarRangeFactor(float Distance) const
{
    // 0 at <= 600, 1 at >= 1200
    const float Near = 600.f;
    const float Far = 1200.f;
    float norm = (Distance - Near) / (Far - Near);
    return SmoothStep(norm);
}

float UEnemyGroupTacticalAI::GetHealthFactor(float NormalizedHealth) const
{
    // Expect 0..1 input, clamp
    return FMath::Clamp(NormalizedHealth, 0.f, 1.f);
}

float UEnemyGroupTacticalAI::GetDangerFactor(float HealthPercent) const
{
    // Danger is high when health is low. Use smooth inverse mapping.
    float h = FMath::Clamp(HealthPercent, 0.f, 1.f);
    return 1.f - SmoothStep(h);
}

float UEnemyGroupTacticalAI::GetPressureFactor(float DamageTaken, float DamageDealt) const
{
    // Returns 0..1 where 1 means taking a lot more damage than dealing
    float Sum = DamageTaken + DamageDealt;
    if (Sum <= KINDA_SMALL_NUMBER) return 0.f;
    float Ratio = DamageTaken / Sum; // 0..1
    return SmoothStep(FMath::Clamp(Ratio, 0.f, 1.f));
}

float UEnemyGroupTacticalAI::GetPressureFactor(int32 GroupSize) const
{
    // Larger groups can apply more pressure. Smooth ramp from 1..8 members.
    const float Min = 1.f;
    const float Max = 8.f;
    float norm = (GroupSize - Min) / (Max - Min);
    return SmoothStep(FMath::Clamp(norm, 0.f, 1.f));
}

float UEnemyGroupTacticalAI::GetSpreadFactor(float AvgDistanceBetweenMembers) const
{
    // 0 at <= 100, 1 at >= 600
    const float Close = 100.f;
    const float Far = 600.f;
    float norm = (AvgDistanceBetweenMembers - Close) / (Far - Close);
    return SmoothStep(FMath::Clamp(norm, 0.f, 1.f));
}

float UEnemyGroupTacticalAI::GetFormationSpreadFactor(float SpreadDistance) const
{
    // Slightly different curve for formation suitability checks
    const float Close = 50.f;
    const float Far = 500.f;
    float norm = (SpreadDistance - Close) / (Far - Close);
    return SmoothStep(FMath::Clamp(norm, 0.f, 1.f));
}

float UEnemyGroupTacticalAI::GetTargetMovementFactor(float TargetVelocity) const
{
    // Normalize against an assumed max useful velocity (e.g. 800)
    const float MaxVel = 800.f;
    return FMath::Clamp(TargetVelocity / MaxVel, 0.f, 1.f);
}

void UEnemyGroupTacticalAI::NotifyCombatEnded(EFormationType UsedFormation, EFormationTuningPreset UsedTuningPreset, ECombatOutcome Outcome, float DamageDealt, float DamageTaken, float CombatDurationSeconds)
{
    const bool bSuccess = Outcome == ECombatOutcome::Victory;
    const float TacticalSuccessScore = CalculateTacticalSuccessScore(Outcome, DamageDealt, DamageTaken, CombatDurationSeconds);

    if (UsedFormation == EFormationType::Circle)
    {
        CircleMomentum = FMath::Clamp(CircleMomentum + TacticalSuccessScore * 0.2f, -2.f, 2.f);
    }
    else if (UsedFormation == EFormationType::Line)
    {
        LineMomentum = FMath::Clamp(LineMomentum + TacticalSuccessScore * 0.2f, -2.f, 2.f);
    }
    else if (UsedFormation == EFormationType::Wedge)
    {
        WedgeMomentum = FMath::Clamp(WedgeMomentum + TacticalSuccessScore * 0.2f, -2.f, 2.f);
    }

    double Now = FPlatformTime::Seconds();
    if (UsedFormation != LastChosenFormation)
    {
        LastChosenFormation = UsedFormation;
        LastFormationChangeTime = Now;
    }

    if (bSuccess)
    {
        LastChosenFormation = UsedFormation;
        RecentSuccessBias = FMath::Clamp(RecentSuccessBias + 0.2f * TacticalSuccessScore, -1.f, 1.f);
    }
    else
    {
        RecentSuccessBias = FMath::Clamp(RecentSuccessBias - 0.15f, -1.f, 1.f);
    }

    if (UEnemyTacticalLearningSubsystem* LearningSubsystem = GetLearningSubsystem())
    {
        LearningSubsystem->SubmitCombatResult(
            LastObservedContext,
            UsedFormation,
            UsedTuningPreset,
            Outcome,
            DamageDealt,
            DamageTaken,
            CombatDurationSeconds
        );
    }

    if (bEnableTacticalDebug)
    {
        UE_LOG(LogTemp, Warning, TEXT("TacticalAI: NotifyCombatEnded Form=%d Preset=%d Success=%d TacticalScore=%.3f MomentumC=%.2f MLine=%.2f MWedge=%.2f"),
            (int)UsedFormation, (int)UsedTuningPreset, (int)bSuccess, TacticalSuccessScore, CircleMomentum, LineMomentum, WedgeMomentum);
    }
}

void UEnemyGroupTacticalAI::SaveLearningData()
{
    if (UEnemyTacticalLearningSubsystem* LearningSubsystem = GetLearningSubsystem())
    {
        LearningSubsystem->SaveLearningData();
    }
}

void UEnemyGroupTacticalAI::LoadLearningData()
{
    if (UEnemyTacticalLearningSubsystem* LearningSubsystem = GetLearningSubsystem())
    {
        CirclePreferenceWeight = LearningSubsystem->GetPreferenceWeight(EFormationType::Circle);
        LinePreferenceWeight = LearningSubsystem->GetPreferenceWeight(EFormationType::Line);
        WedgePreferenceWeight = LearningSubsystem->GetPreferenceWeight(EFormationType::Wedge);
    }
}

UEnemyTacticalLearningSubsystem* UEnemyGroupTacticalAI::GetLearningSubsystem() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        return nullptr;
    }

    return GameInstance->GetSubsystem<UEnemyTacticalLearningSubsystem>();
}

float UEnemyGroupTacticalAI::CalculateTacticalSuccessScore(
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
