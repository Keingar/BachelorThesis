// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlockInfo.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FBlockInfo
{
	GENERATED_BODY()

public:
	FBlockInfo()
		: PhysResistance(0.0f)
		, MagicResistance(0.0f)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
	float PhysResistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
	float MagicResistance;
};
