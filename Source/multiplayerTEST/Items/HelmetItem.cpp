// Fill out your copyright notice in the Description page of Project Settings.


#include "HelmetItem.h"
#include "multiplayerTEST/Components/InventoryComponent.h"

UHelmetItem::UHelmetItem()
{
	ItemSubCategory = EItemSubCategory::LightHelmet;
}

void UHelmetItem::Use()
{
	if (!OwningInventory) return;

	if (bIsEquipped) {
		OwningInventory->TakeOffHelmet();
		return;
	}

	OwningInventory->EquipHelmet(this);
}