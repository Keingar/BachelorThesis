// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemsWidget.h"
#include "multiplayerTEST/Items/Item.h"

void UItemsWidget::OnItemClicked()
{
	Item->UseItemFromInterface();
}
