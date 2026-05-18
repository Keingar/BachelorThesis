// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "multiplayerTEST/CustomStructs/BlockInfo.h"
#include "WearableItem.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UWearableItem : public UItem
{
	GENERATED_BODY()
	
public:
	UWearableItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 0.0))
	float Weight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector RelativeLocationOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRotator RelativeRotationOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector ScaleWhenEquipped;

	UFUNCTION(BlueprintCallable)
	FBlockInfo GetItemDamageResistance() { return ItemDamageResistance; }

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* ItemMaskTexture;

	UPROPERTY(BlueprintreadOnly)
	bool bIsEquipped;

	// used only in UI
	UPROPERTY(BlueprintReadOnly)
	int EquippedSlot;
protected:
	UPROPERTY(EditDefaultsOnly)
	FBlockInfo ItemDamageResistance;
};
