// Fill out your copyright notice in the Description page of Project Settings.


#include "ChestItem.h"
#include "multiplayerTEST/Components/InventoryComponent.h"

UChestItem::UChestItem()
{
	ItemSubCategory = EItemSubCategory::LightChest;
}

void UChestItem::Use()
{
	if (!OwningInventory) return;

	if (bIsEquipped) {
		OwningInventory->TakeOffChest();
		return;
	}

	OwningInventory->EquipChest(this);
}