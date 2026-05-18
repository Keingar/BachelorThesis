#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FormationPerformance.h"
#include "TacticalContextKey.h"
#include "EnemyTacticalAISaveGame.generated.h"

UCLASS()
class MULTIPLAYERTEST_API UEnemyTacticalAISaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // Preference weights
    UPROPERTY()
    float CirclePreferenceWeight = 1.f;

    UPROPERTY()
    float LinePreferenceWeight = 1.f;

    UPROPERTY()
    float WedgePreferenceWeight = 1.f;

    // Momentum values
    UPROPERTY()
    float CircleMomentum = 0.f;

    UPROPERTY()
    float LineMomentum = 0.f;

    // Contextual performance map saved as a native UPROPERTY map
    UPROPERTY()
    TMap<FTacticalContextKey, FFormationPerformanceData> ContextPerformanceMap;

    UPROPERTY()
    TMap<FTacticalTuningContextKey, FFormationPerformanceData> TuningPerformanceMap;

    UPROPERTY()
    TMap<FName, float> NeuroFuzzyRuleConsequentOffsets;

    // Tactical state value saved as uint8 (stores ETacticalState)
    UPROPERTY()
    uint8 LastTacticalState = 0;

    // Save format version for compatibility
    UPROPERTY()
    int32 SaveVersion = 4;
};
