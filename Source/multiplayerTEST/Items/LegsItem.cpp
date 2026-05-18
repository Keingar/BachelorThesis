// Fill out your copyright notice in the Description page of Project Settings.


#include "LegsItem.h"
#include "multiplayerTEST/Components/InventoryComponent.h"

ULegsItem::ULegsItem()
{
	ItemSubCategory = EItemSubCategory::LightLegs;
}

void ULegsItem::Use()
{
	if (!OwningInventory) return;

	if (bIsEquipped) {
		OwningInventory->TakeOffLegs();
		return;
	}

	OwningInventory->EquipLegs(this);
}