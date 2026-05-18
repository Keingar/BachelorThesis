// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WearableItem.h"
#include "ChestItem.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UChestItem : public UWearableItem
{
	GENERATED_BODY()

public:

	UChestItem();

	virtual void Use() override;
};
