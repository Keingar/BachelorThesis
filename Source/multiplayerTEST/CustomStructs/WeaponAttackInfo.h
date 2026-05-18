// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponPatternInfo.h"
#include "GameplayTagContainer.h"
#include "WeaponAttackInfo.generated.h"

class UCoreAbility;

USTRUCT(BlueprintType)
struct FWeaponAttackInfo
{
	GENERATED_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FWeaponPatternInfo> WeaponPatterns;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<int> attackAnimationSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UCoreAbility>  AbilityClass;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ComboResetTag;
};