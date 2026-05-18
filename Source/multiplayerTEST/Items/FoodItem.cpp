// Fill out your copyright notice in the Description page of Project Settings.


#include "FoodItem.h"
#include "multiplayerTEST/Components/InventoryComponent.h"
#include "multiplayerTEST/Components/HealthComponent.h"

UFoodItem::UFoodItem()
{
	ItemSubCategory = EItemSubCategory::Bombs;
}


void UFoodItem::Use()
{
	if (!this || !OwningInventory || OwningInventory->AvatarActor) return;
	
	if (UHealthComponent* HealthComponent = OwningInventory->AvatarActor->GetComponentByClass<UHealthComponent>()) {
		HealthComponent->Heal(HealthToHeal);
	}
	

	Quantity = Quantity - 1;
	OwningInventory->RemoveItem(this);
}