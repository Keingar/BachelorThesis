#pragma once

#include "CoreMinimal.h"
#include "ImpactInfo.generated.h"

USTRUCT(BlueprintType)
struct FImpactInfo
{
    GENERATED_BODY()

public:

    // Inline constructor
    FImpactInfo()
        : PoiseDamage(0.f)
		, StaminaDamageOnBlock(0.f)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImpactInfo")
    float PoiseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImpactInfo")
    float StaminaDamageOnBlock;

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "ImpactInfo",
        meta=(ToolTip ="If its nullptr then impacts from impactComponent will be used otherwise this will override any other impact"))
	UAnimMontage* CustomImpactMontage = nullptr; // this can override ImpactComponent impacts
};
