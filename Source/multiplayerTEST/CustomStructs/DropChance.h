// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DropChance.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FDropChance
{
	GENERATED_BODY()

public:
	FDropChance()
		: ItemID(0)
		, DropChance(0.f)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DropChance")
	int ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DropChance")
	float DropChance;
};
