// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "multiplayerTEST/Components/InventoryComponent.h"

UWeapon::UWeapon()
{
	ItemSubCategory = EItemSubCategory::LongSword;

	LightAttackInfo.ComboResetTag = FGameplayTag::RequestGameplayTag(FName("StatusEffect.ComboAnimStatus.CanLightAttackAgain"));
	HeavyAttackInfo.ComboResetTag = FGameplayTag::RequestGameplayTag(FName("StatusEffect.ComboAnimStatus.CanHeavyAttackAgain"));
	DoubleLightAttackInfo.ComboResetTag = FGameplayTag::RequestGameplayTag(FName("StatusEffect.ComboAnimStatus.CanDoubleLightAttackAgain"));
	TH_LightAttackInfo.ComboResetTag = FGameplayTag::RequestGameplayTag(FName("StatusEffect.ComboAnimStatus.CanTHLightAttackAgain"));
	RollAttackInfo.ComboResetTag = FGameplayTag::RequestGameplayTag(FName("StatusEffect.ComboAnimStatus.CanRollAttackAgain"));

}

void UWeapon::Use()
{
	if (!OwningInventory) return;

	if (bIsEquipped) {
		OwningInventory->TakeOffWeaponRH1();
		return;
	}

	OwningInventory->EquipWeaponRH1(this);
}