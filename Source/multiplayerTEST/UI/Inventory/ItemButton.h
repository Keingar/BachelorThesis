// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "multiplayerTEST/Customenums/ItemSubCategory.h"
#include "ItemButton.generated.h"

class UWearableItem;

UCLASS()
class MULTIPLAYERTEST_API UItemButton : public UButton
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	UWearableItem* relatedItem;

	TEnumAsByte<EItemSubCategory> RelatedCategory; // needed anyway when item is nullptr
};
