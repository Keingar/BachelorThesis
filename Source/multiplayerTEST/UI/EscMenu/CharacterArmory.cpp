// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterArmory.h"
#include "multiplayerTEST/Items/WearableItem.h"
#include "Components/Overlay.h"
#include "Components/Button.h"
#include "Components/WrapBox.h"
#include "Components/HorizontalBox.h"
#include "multiplayerTEST/Components/InventoryComponent.h"
#include "multiplayerTEST/UI/Inventory/ItemsWidget.h"
#include "multiplayerTEST/UI/Inventory/ItemButton.h"
#include "multiplayerTEST/UI/Inventory/InventoryWidget.h"

void UCharacterArmory::NativeConstruct()
{
    Super::NativeConstruct();
    ArmoryInventoryWidget->OwningArmory = this;
}

void UCharacterArmory::SetUpArmory()
{
    if (!ArmoryInventoryWidget) return;

    UInventoryComponent* InvComp = ArmoryInventoryWidget->InventoryComponent;

    if (!InvComp) return;

    SetupEquippedSlot(HelmetItemButton, InvComp->SaveeHelmet);
    SetupEquippedSlot(ChestItemButton, InvComp->SaveChest);
    SetupEquippedSlot(LegsItemButton, InvComp->SaveLegs);
    SetupEquippedSlot(WeaponRH1Button, InvComp->SaveWeaponRH1);
    SetupEquippedSlot(WeaponLH1Button, InvComp->SaveWeaponLH1);
    SetupEquippedSlot(HeartButton, InvComp->SaveHeartItem);
}

void UCharacterArmory::SetupEquippedSlot(
    UItemButton* Button,
    UWearableItem* EquippedItem
)
{
    if (!Button || !EquippedItem)
    {
        return;
    }

    Button->relatedItem = EquippedItem;
    SetUIEquippedItemForGivenButton(Button);
}

void UCharacterArmory::UpdateCorrectArmory(UWearableItem* CurrentItem)
{
    if (!CurrentItem) {
        return;
    }

    UItemsWidget* ItemWidget = CreateWidget<UItemsWidget>(GetOwningPlayer(), ArmoryItemBlueprintWidget);
    ItemWidget->RelatedOwningWidget = this;
    ItemWidget->Item = CurrentItem;

    // check if equpped and same as last clicked button item to set bool

    ItemWidget->InitiolazeItem();
}


void UCharacterArmory::OnSelectEquipItem(UItem* ItemToEquip)
{
    if (!InventoryComponent || !LastClickedButton) return;

    UWearableItem* WearableItemToEquip = Cast<UWearableItem>(ItemToEquip);
    if (!WearableItemToEquip) return;

    if (WearableItemToEquip->bIsEquipped && ItemToEquip != LastClickedButton->relatedItem) {
        // take off from its slot and equip to new one
        TakeOffItemUsingCategory(WearableItemToEquip);
        
        EquipItem(WearableItemToEquip);

        return;
    }

    if (WearableItemToEquip->bIsEquipped) {
        TakeOffHoveredItem();
        return;
    }

    EquipItem(WearableItemToEquip);
}

void UCharacterArmory::EquipItem(UWearableItem* ItemToEquip) {
    LastClickedButton->relatedItem = ItemToEquip;

    SetUIEquippedItem();

   // ArmoryInventoryWidget->UpdateCorrectWrapBoxWithEquipChecks(ItemToEquip->ItemSubCategory, LastClickedButton);
    CallCorrectEquipFunction();
}

void UCharacterArmory::SetUpButtonsRelationToInventory(UInventoryComponent* newInventoryComponent)
{
    InventoryComponent = newInventoryComponent;

	HelmetItemButton->RelatedCategory = EItemSubCategory::LightHelmet;
	ChestItemButton->RelatedCategory = EItemSubCategory::LightChest;
    LegsItemButton->RelatedCategory = EItemSubCategory::LightLegs;
    WeaponRH1Button->RelatedCategory = EItemSubCategory::LongSword;
    WeaponLH1Button->RelatedCategory = EItemSubCategory::LongSword;
    HeartButton->RelatedCategory = EItemSubCategory::Heart;

    ArmoryInventoryWidget->OwningArmory = this;
    ArmoryInventoryWidget->InitializeInventory(newInventoryComponent);

    SetUpArmory();

}

void UCharacterArmory::ResetArmoryUI()
{
    if (Armory->GetVisibility() != ESlateVisibility::Hidden) return;

    ArmoryInventoryBox->SetVisibility(ESlateVisibility::Hidden);
    Armory->SetVisibility(ESlateVisibility::Visible);
    LastClickedButton = nullptr;
   // ItemListWrapBox->ClearChildren();
}

void UCharacterArmory::OnItemButtonPressed(UItemButton* NewLastClickedButton)
{
    APlayerController* SaveController = GetOwningPlayer();

    if (!InventoryComponent || !NewLastClickedButton || !SaveController) return;
        
    LastClickedButton = NewLastClickedButton;

    ArmoryInventoryWidget->ShowGivenCategoryInventoryTabWithOtherSlotEquipChecks(NewLastClickedButton->RelatedCategory, NewLastClickedButton);

    ArmoryInventoryBox->SetVisibility(ESlateVisibility::Visible);
    Armory->SetVisibility(ESlateVisibility::Hidden);

}

void UCharacterArmory::CallCorrectEquipFunction()
{
    if (LastClickedButton == HelmetItemButton)
    {
        InventoryComponent->EquipHelmet(LastClickedButton->relatedItem);
    }
    else if (LastClickedButton == ChestItemButton)
    {
        InventoryComponent->EquipChest(LastClickedButton->relatedItem);
    }
    else if (LastClickedButton == LegsItemButton)
    {
        InventoryComponent->EquipLegs(LastClickedButton->relatedItem);
    }
    else if (LastClickedButton == WeaponRH1Button)
    {
        InventoryComponent->EquipWeaponRH1(LastClickedButton->relatedItem);
    }
    else if (LastClickedButton == WeaponLH1Button)
    {
        InventoryComponent->EquipWeaponLH1(LastClickedButton->relatedItem);
    }
    else if (LastClickedButton == HeartButton)
    {
        InventoryComponent->EquipHeart(LastClickedButton->relatedItem);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Not valid last clicked button for equipping! Button ptr: %p"), LastClickedButton);
    }
}

void UCharacterArmory::TakeOffHoveredItem()
{
    if (CurrentHoveredButton) {
		TakeOffHoveredButtonItem();
        return;
    }

    if (CurrentHoveredItemWidget) {
        TakeOffHoveredItemWidgetItem();
		return;
    }
}

void UCharacterArmory::TakeOffHoveredButtonItem() {
    if (!CurrentHoveredButton || !InventoryComponent || !CurrentHoveredButton->relatedItem) return;

    UItemButton* TempSaveofHoveredButton = CurrentHoveredButton; // to make sure it doesn't change in the process

    if (TempSaveofHoveredButton == HelmetItemButton)
    {
        InventoryComponent->TakeOffHelmet();
    }
    else if (TempSaveofHoveredButton == ChestItemButton)
    {
        InventoryComponent->TakeOffChest();
    }
    else if (TempSaveofHoveredButton == LegsItemButton)
    {
        InventoryComponent->TakeOffLegs();
    }
    else if (TempSaveofHoveredButton == WeaponRH1Button)
    {
        InventoryComponent->TakeOffWeaponRH1();
    }
    else if (TempSaveofHoveredButton == WeaponLH1Button)
    {
        InventoryComponent->TakeOffWeaponLH1();
    }
    else if (TempSaveofHoveredButton == HeartButton)
    {
        InventoryComponent->TakeOffHeart();
    }

    UpdateGivenButtonUIForTakeOff(TempSaveofHoveredButton);

    TempSaveofHoveredButton->relatedItem = nullptr;
}

void UCharacterArmory::TakeOffHoveredItemWidgetItem() {
    TakeOffItemUsingCategory(CurrentHoveredItemWidget->Item);
}

void UCharacterArmory::TakeOffItemUsingCategory(UItem* ItemToTakeOff)
{
    if (!ItemToTakeOff || !InventoryComponent) return;

    switch (ItemToTakeOff->ItemSubCategory)
    {
    case EItemSubCategory::LightHelmet:
        InventoryComponent->TakeOffHelmet();
        UpdateGivenButtonUIForTakeOff(HelmetItemButton);

        break;
    case EItemSubCategory::LightChest:
        InventoryComponent->TakeOffChest();
        UpdateGivenButtonUIForTakeOff(ChestItemButton);

        break;
    case EItemSubCategory::LightLegs:
        InventoryComponent->TakeOffLegs();
        UpdateGivenButtonUIForTakeOff(LegsItemButton);

        break;
    case EItemSubCategory::LongSword:
    case EItemSubCategory::Shield:
    case EItemSubCategory::Polearm:
        if (InventoryComponent->SaveWeaponRH1 == ItemToTakeOff) {
            InventoryComponent->TakeOffWeaponRH1();
            UpdateGivenButtonUIForTakeOff(WeaponRH1Button);

        }
        else if (InventoryComponent->SaveWeaponLH1 == ItemToTakeOff) {
            InventoryComponent->TakeOffWeaponLH1();
            UpdateGivenButtonUIForTakeOff(WeaponLH1Button);

        }
        break;
    case EItemSubCategory::Heart:
        InventoryComponent->TakeOffHeart();
		UpdateGivenButtonUIForTakeOff(HeartButton);

        break;
    default:
        UE_LOG(LogTemp, Error, TEXT("Not valid item subcategory for take off! Subcategory value: %d"), ItemToTakeOff->ItemSubCategory);
        break;
    }
}

void UCharacterArmory::SetCurrentHoveredWidget(UItemsWidget* HoveredItemWidget)
{
    CurrentHoveredItemWidget = HoveredItemWidget;
}
