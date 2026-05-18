// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WearableItem.h"
#include "LegsItem.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API ULegsItem : public UWearableItem
{
	GENERATED_BODY()

public:

	ULegsItem();

	virtual void Use() override;
};
