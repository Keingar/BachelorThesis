#pragma once

#include "CoreMinimal.h"
#include "FormationDecision.h"
#include "TacticalContextKey.generated.h"

USTRUCT(BlueprintType)
struct FTacticalContextKey
{
    GENERATED_BODY()

    UPROPERTY()
    int32 GroupSizeBucket = 0;

    UPROPERTY()
    int32 DistanceBucket = 0;

    UPROPERTY()
    bool bTargetIsEnemyGroup = false;

    UPROPERTY()
    int32 TargetGroupSizeBucket = 0;

    UPROPERTY()
    EFormationType FormationType = EFormationType::None;

    bool operator==(const FTacticalContextKey& Other) const
    {
        return GroupSizeBucket == Other.GroupSizeBucket
            && DistanceBucket == Other.DistanceBucket
            && bTargetIsEnemyGroup == Other.bTargetIsEnemyGroup
            && TargetGroupSizeBucket == Other.TargetGroupSizeBucket
            && FormationType == Other.FormationType;
    }
};

FORCEINLINE uint32 GetTypeHash(const FTacticalContextKey& Key)
{
    uint32 Hash = ::GetTypeHash(Key.GroupSizeBucket);
    Hash = HashCombine(Hash, ::GetTypeHash(Key.DistanceBucket));
    Hash = HashCombine(Hash, ::GetTypeHash(Key.bTargetIsEnemyGroup ? 1 : 0));
    Hash = HashCombine(Hash, ::GetTypeHash(Key.TargetGroupSizeBucket));
    Hash = HashCombine(Hash, ::GetTypeHash((int32)Key.FormationType));
    return Hash;
}

USTRUCT(BlueprintType)
struct FTacticalTuningContextKey
{
    GENERATED_BODY()

    UPROPERTY()
    FTacticalContextKey ContextKey;

    UPROPERTY()
    EFormationTuningPreset TuningPreset = EFormationTuningPreset::Balanced;

    bool operator==(const FTacticalTuningContextKey& Other) const
    {
        return ContextKey == Other.ContextKey
            && TuningPreset == Other.TuningPreset;
    }
};

FORCEINLINE uint32 GetTypeHash(const FTacticalTuningContextKey& Key)
{
    uint32 Hash = GetTypeHash(Key.ContextKey);
    Hash = HashCombine(Hash, ::GetTypeHash((int32)Key.TuningPreset));
    return Hash;
}
