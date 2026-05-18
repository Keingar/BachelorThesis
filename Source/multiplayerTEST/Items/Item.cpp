// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

UItem::UItem()
{
	ItemDisplayName = FText::FromString("Item");
	UseActionText = FText::FromString("Use");
	Quantity = 1;

	ItemSubCategory = EItemSubCategory::Bombs;
}

bool UItem::CanBeUsed()
{
	return true;
}

bool UItem::UseItemFromInterface() {
	return CanBeUsed() && (Use(), OnUse(), true);
}