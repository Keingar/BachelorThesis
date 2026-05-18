// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterArmory.generated.h"

class UItemButton;
class UOverlay;
class UHorizontalBox;
class UInventoryWidget;
class UItemsWidget;
class UWearableItem;
class UItem;

UCLASS()
class MULTIPLAYERTEST_API UCharacterArmory : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UItemButton* HelmetItemButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UItemButton* ChestItemButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UItemButton* LegsItemButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UItemButton* WeaponRH1Button;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UItemButton* WeaponLH1Button;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UItemButton* HeartButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Armory;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* ArmoryInventoryBox;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UInventoryWidget* ArmoryInventoryWidget;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UItemsWidget> ArmoryItemBlueprintWidget;

	UPROPERTY(BlueprintReadOnly)
	UItemButton* LastClickedButton;
	UPROPERTY(BlueprintReadWrite)
	UItemButton* CurrentHoveredButton;
	UItemsWidget* CurrentHoveredItemWidget;
public:
	virtual void NativeConstruct() override;

	void UpdateCorrectArmory(UWearableItem* CurrentItem);

	class UInventoryComponent* InventoryComponent;
	UFUNCTION(BlueprintCallable)
	void OnSelectEquipItem(UItem* ItemToEquip);

	UFUNCTION(BlueprintCallable)
	void SetUpButtonsRelationToInventory(UInventoryComponent* newInventoryComponent);

	UFUNCTION(BlueprintCallable)
	void ResetArmoryUI(); // for situations when ESC was pressed again while in inventory section

	UFUNCTION(BlueprintCallable)
	void TakeOffHoveredItem();

	UFUNCTION(BlueprintCallable)
	void SetCurrentHoveredWidget(UItemsWidget* HoveredItemWidget);

	UItemButton* GetLastClickedButton() const { return LastClickedButton; }
protected:

	UFUNCTION(BlueprintCallable)
	void OnItemButtonPressed(UItemButton* NewLastClickedButton);

	UFUNCTION(BlueprintImplementableEvent)
	void SetUIEquippedItem();
	UFUNCTION(BlueprintImplementableEvent)
	void SetUIEquippedItemForGivenButton(UItemButton* ButtonToSetUp);

	void CallCorrectEquipFunction();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateGivenButtonUIForTakeOff(UItemButton* GivenHoveredButton);

	void TakeOffHoveredButtonItem();
	void TakeOffHoveredItemWidgetItem();
	void TakeOffItemUsingCategory(UItem* ItemToTakeOff);

	void EquipItem(UWearableItem* ItemToEquip);
	void SetUpArmory();

	void SetupEquippedSlot(UItemButton* Button, UWearableItem* EquippedItem);
};
