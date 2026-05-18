// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "FoodItem.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UFoodItem : public UItem
{
	GENERATED_BODY()
	
public:
	UFoodItem();

	virtual void Use() override;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 0.0))
	float HealthToHeal;
};
