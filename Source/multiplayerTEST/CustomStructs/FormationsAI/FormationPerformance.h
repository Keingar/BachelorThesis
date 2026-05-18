#pragma once

#include "CoreMinimal.h"
#include "FormationPerformance.generated.h"

USTRUCT(BlueprintType)
struct FFormationPerformanceData
{
    GENERATED_BODY()

    // Cumulative success score: higher means better historical performance
    UPROPERTY(BlueprintReadWrite)
    float SuccessScore = 0.f;

    UPROPERTY(BlueprintReadWrite)
    int32 TimesUsed = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 Victories = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 Failures = 0;

    UPROPERTY(BlueprintReadWrite)
    float AverageDamageDealt = 0.f;

    UPROPERTY(BlueprintReadWrite)
    float AverageDamageTaken = 0.f;

    UPROPERTY(BlueprintReadWrite)
    float AverageCombatDuration = 0.f;
};
