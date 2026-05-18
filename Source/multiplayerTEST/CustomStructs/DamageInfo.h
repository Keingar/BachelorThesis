#pragma once

#include "CoreMinimal.h"
#include "ImpactInfo.h"
#include "DamageInfo.generated.h"

USTRUCT(BlueprintType)
struct FDamageInfo
{
    GENERATED_BODY()

public:

    // Inline constructor
    FDamageInfo()
        : PhysDamage(0.f)
        , MagicDamage(0.f)
        , DamagePercent(1.f)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
    float PhysDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
    float MagicDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
    float DamagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
    FImpactInfo ImpactInfo;
};
