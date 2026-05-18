// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WearableItem.h"
#include "HelmetItem.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UHelmetItem : public UWearableItem
{
	GENERATED_BODY()

public:

	UHelmetItem();

	virtual void Use() override;
};
