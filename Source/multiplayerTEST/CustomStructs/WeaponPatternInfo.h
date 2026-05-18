// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponPatternInfo.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FWeaponPatternInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAnimMontage* AttackAnimation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamagePercent = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaminaConsumption = 0.f;
};