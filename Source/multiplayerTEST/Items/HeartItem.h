// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WearableItem.h"
#include "HeartItem.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UHeartItem : public UWearableItem
{
	GENERATED_BODY()

public:

	UHeartItem();

	virtual void Use() override;
};
