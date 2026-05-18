// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RollBullshitData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MULTIPLAYERTEST_API URollBullshitData : public UObject
{
	GENERATED_BODY()
	

public:
	UPROPERTY(BlueprintReadOnly)
	double XThingy;
	UPROPERTY(BlueprintReadOnly)
	double YThingy;
};
