// Fill out your copyright notice in the Description page of Project Settings.


#include "HeartItem.h"
#include "multiplayerTEST/Components/InventoryComponent.h"

UHeartItem::UHeartItem()
{
	ItemSubCategory = EItemSubCategory::Heart;
}

void UHeartItem::Use()
{
	if (!OwningInventory) return;
	
	if (bIsEquipped) {
		OwningInventory->TakeOffHeart();
		return;
	}

	OwningInventory->EquipHeart(this);
}