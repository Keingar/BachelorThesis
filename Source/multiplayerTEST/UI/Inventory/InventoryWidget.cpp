// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include "Components/WrapBox.h"
#include "Components/ScrollBox.h"
#include "Components/Overlay.h"
#include "multiplayerTEST/Components/InventoryComponent.h"
#include "ItemsWidget.h"
#include "GameFramework/PlayerState.h"
#include "multiplayerTEST/Items/WearableItem.h"
#include "ItemButton.h"
#include "multiplayerTEST/UI/EscMenu/CharacterArmory.h"

void UInventoryWidget::InitializeInventory(UInventoryComponent* newInventoryComponent)
{
    InventoryComponent = newInventoryComponent;

    InventoryComponent->OnUpdatedInventory.AddUObject(this, &UInventoryWidget::UpdateCorrectWrapBox);

    UpdateCorrectInventory(&InventoryComponent->MiscItems);
    UpdateCorrectInventory(&InventoryComponent->HelmetItems);
    UpdateCorrectInventory(&InventoryComponent->ChestItems);
    UpdateCorrectInventory(&InventoryComponent->LegsItems);
    UpdateCorrectInventory(&InventoryComponent->WeaponItems);

    ScrollBoxStuff->SetVisibility(ESlateVisibility::Visible);
    ScrollBoxChest->SetVisibility(ESlateVisibility::Hidden);
    ScrollBoxHelmet->SetVisibility(ESlateVisibility::Hidden);
    ScrollBoxLegs->SetVisibility(ESlateVisibility::Hidden);
    ScrollBoxWeapons->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryWidget::UpdateCorrectInventory(TArray<class UItem*>* CurrentItems)
{
    APlayerController* SaveController = GetOwningPlayer();

    if (!CurrentItems || !SaveController) return;

    TSet<UWrapBox*> ClearedWrapBoxes; // track which wrap boxes we've cleared

    for (UItem* Item : *CurrentItems)
    {
        if (isArmoryInventory) {
            UWearableItem* CheckIfWearable = Cast<UWearableItem>(Item);
            if (!CheckIfWearable) { continue; }
        }

        UWrapBox* CorrectWrapBox = GetWrapBoxBySubCategory(Item->ItemSubCategory);
        if (!CorrectWrapBox) continue;

        if (!ClearedWrapBoxes.Contains(CorrectWrapBox))
        {
            CorrectWrapBox->ClearChildren();
            ClearedWrapBoxes.Add(CorrectWrapBox);
        }

        UItemsWidget* SaveWidgetItem = CreateWidget<UItemsWidget>(SaveController, ItemBlueprintWidget);

        SaveWidgetItem->Item = Item;

        if (isArmoryInventory) {
            SaveWidgetItem->RelatedOwningWidget = OwningArmory;
        }
        else {
            SaveWidgetItem->RelatedOwningWidget = this;
        }

        SaveWidgetItem->InitiolazeItem();
        CorrectWrapBox->AddChildToWrapBox(SaveWidgetItem);
    }
}

void UInventoryWidget::UpdateCorrectWrapBox(TEnumAsByte<EItemSubCategory> Category)
{
    APlayerController* SaveController = GetOwningPlayer();

    UWrapBox* CorrectWrapBox = GetWrapBoxBySubCategory(Category);

    if (!CorrectWrapBox || !SaveController || !InventoryComponent) return;

    if (isArmoryInventory &&
        (CorrectWrapBox == PolearmItems ||
            CorrectWrapBox == LongSwordItems ||
            CorrectWrapBox == ShieldBoxItems))

    {
        UCharacterArmory* OwnerWidget = Cast<UCharacterArmory>(OwningArmory);
        if (!OwnerWidget) return;
        UpdateCorrectWrapBoxWithEquipChecks(Category, OwnerWidget->GetLastClickedButton());
        return;
    }

    CorrectWrapBox->ClearChildren();
    for (UItem* Item : *InventoryComponent->GetItemArrayFromItemSubCategory(Category))
    {
        if (Item->ItemSubCategory != Category) continue;

        UItemsWidget* SaveWidgetItem = CreateWidget<UItemsWidget>(SaveController, ItemBlueprintWidget);

        SaveWidgetItem->Item = Item;

        if (isArmoryInventory) {
            SaveWidgetItem->RelatedOwningWidget = OwningArmory;

        }
        else {
            SaveWidgetItem->RelatedOwningWidget = this;
        }

        SaveWidgetItem->InitiolazeItem();
        CorrectWrapBox->AddChildToWrapBox(SaveWidgetItem);
    }
}

void UInventoryWidget::UpdateCorrectWrapBoxWithEquipChecks(TEnumAsByte<EItemSubCategory> Category, UItemButton* LastClickedButtonInArmory)
{
    if (!isArmoryInventory) {
		UE_LOG(LogTemp, Error, TEXT("Was called armory update function for non-armory inventory widget!"));
        return;
    }

    APlayerController* SaveController = GetOwningPlayer();

    UWrapBox* CorrectWrapBox = GetWrapBoxBySubCategory(Category);

    if (!CorrectWrapBox || !SaveController || !InventoryComponent) return;

    CorrectWrapBox->ClearChildren();
    for (UItem* Item : *InventoryComponent->GetItemArrayFromItemSubCategory(Category))
    {
        if (Item->ItemSubCategory != Category) continue;

        UItemsWidget* SaveWidgetItem = CreateWidget<UItemsWidget>(SaveController, ItemBlueprintWidget);

        SaveWidgetItem->Item = Item;

        SaveWidgetItem->RelatedOwningWidget = OwningArmory;

        SaveWidgetItem->InitiolazeItem();
        CorrectWrapBox->AddChildToWrapBox(SaveWidgetItem);
    }
    UpdateWrapBoxForisEquippedIcon(CorrectWrapBox, LastClickedButtonInArmory);
}

void UInventoryWidget::ShowGivenCategoryInventoryTab(TEnumAsByte<EItemSubCategory> CategoryToShow)
{
    UScrollBox* CorrectScrollBox = GetScrollBoxByCategory(CategoryToShow);

    if (!CorrectScrollBox) return;
    ScrollBoxStuff->SetVisibility(ESlateVisibility::Hidden);
    ScrollBoxChest->SetVisibility(ESlateVisibility::Hidden);
    ScrollBoxHelmet->SetVisibility(ESlateVisibility::Hidden);
    ScrollBoxLegs->SetVisibility(ESlateVisibility::Hidden);
    ScrollBoxWeapons->SetVisibility(ESlateVisibility::Hidden);

    CorrectScrollBox->SetVisibility(ESlateVisibility::Visible);
}

void UInventoryWidget::ShowGivenCategoryInventoryTabWithOtherSlotEquipChecks(TEnumAsByte<EItemSubCategory> CategoryToShow, UItemButton* LastClickedButtonInArmory)
{
    UScrollBox* CorrectScrollBox = GetScrollBoxByCategory(CategoryToShow);
  //  ItemWidget->bEquippedNotFromThisSlot = CurrentItem->bIsEquipped && LastClickedButton->relatedItem != CurrentItem;

    if (!CorrectScrollBox) return;
    ScrollBoxStuff->SetVisibility(ESlateVisibility::Hidden);
    ScrollBoxChest->SetVisibility(ESlateVisibility::Hidden);
    ScrollBoxHelmet->SetVisibility(ESlateVisibility::Hidden);
    ScrollBoxLegs->SetVisibility(ESlateVisibility::Hidden);
    ScrollBoxWeapons->SetVisibility(ESlateVisibility::Hidden);

    if (CorrectScrollBox != ScrollBoxWeapons) {
        CorrectScrollBox->SetVisibility(ESlateVisibility::Visible);
        return;
    }

    for (UWidget* ScrollChild : ScrollBoxWeapons->GetAllChildren())
    {
        UWrapBox* WrapBox = Cast<UWrapBox>(ScrollChild);
        if (!WrapBox) continue;

        UpdateWrapBoxForisEquippedIcon(WrapBox, LastClickedButtonInArmory);
    }

    CorrectScrollBox->SetVisibility(ESlateVisibility::Visible);
}

UWrapBox* UInventoryWidget::GetWrapBoxBySubCategory(TEnumAsByte<EItemSubCategory> Category)
{
    switch (Category)
    {
    case EItemSubCategory::LightChest:
        return ChestItemsBox;
    case EItemSubCategory::LightHelmet:
        return HelmetItemsBox;
    case EItemSubCategory::LightLegs:
        return LegsItemsBox;
    case EItemSubCategory::Polearm:
        return PolearmItems;
    case EItemSubCategory::LongSword:
        return LongSwordItems;
    case EItemSubCategory::Bombs:
        return BombsItemsBox;
    case EItemSubCategory::Shield:
        return ShieldBoxItems;
    case EItemSubCategory::Heart:
        return HeartItemsBox;
    }
	UE_LOG(LogTemp, Error, TEXT("Wasn't found corresponding WrapBox in GetWrapBoxBySubCategory"));
    return BombsItemsBox;
}

UScrollBox* UInventoryWidget::GetScrollBoxByCategory(TEnumAsByte<EItemSubCategory> Category)
{
    switch (Category)
    {
    case EItemSubCategory::LightChest:
        return ScrollBoxChest;
    case EItemSubCategory::LightHelmet:
        return ScrollBoxHelmet;
    case EItemSubCategory::LightLegs:
        return ScrollBoxLegs;
    case EItemSubCategory::Polearm:
        return ScrollBoxWeapons;
    case EItemSubCategory::LongSword:
        return ScrollBoxWeapons;
    case EItemSubCategory::Bombs:
        return ScrollBoxStuff;
    case EItemSubCategory::Shield:
        return ScrollBoxWeapons;
    case EItemSubCategory::Heart:
        return ScrollBoxStuff;
    }
    UE_LOG(LogTemp, Error, TEXT("Wasn't found corresponding ScrollBox in GetScrollBoxByCategory"));
    return ScrollBoxStuff;
}

void UInventoryWidget::UpdateWrapBoxForisEquippedIcon(UWrapBox* WrapBoxToUpdate, UItemButton* LastClickedButtonInArmory)
{
    if (!LastClickedButtonInArmory) return;

    for (UWidget* WrapChild : WrapBoxToUpdate->GetAllChildren())
    {
        UItemsWidget* ItemWidget = Cast<UItemsWidget>(WrapChild);
        if (!ItemWidget) continue;

        UWearableItem* WearableItem = Cast<UWearableItem>(ItemWidget->Item);

        if (!WearableItem->bIsEquipped) {
            ItemWidget->bEquippedNotFromThisSlot = false;
            continue;
        }

        // was not from this slot before but is now
        if (ItemWidget->bEquippedNotFromThisSlot && ItemWidget->Item == LastClickedButtonInArmory->relatedItem) {
            ItemWidget->bEquippedNotFromThisSlot = false;
            ItemWidget->UpdateIsFromThisSlotUI(true);
            continue;
        }

        if (!WearableItem) continue;

        // not from this slot but was before
        if (!ItemWidget->bEquippedNotFromThisSlot && WearableItem != LastClickedButtonInArmory->relatedItem) {
            ItemWidget->bEquippedNotFromThisSlot = true;
            ItemWidget->UpdateIsFromThisSlotUI(false);
            continue;
        }
    }
}