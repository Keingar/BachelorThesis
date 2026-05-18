// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "multiplayerTEST/CustomEnums/ItemSubCategory.h"
#include "InventoryWidget.generated.h"

class UWrapBox;
class UScrollBox;
class UInventoryComponent;
class UItem;
class UItemButton;
class UItemsWidget;

UCLASS()
class MULTIPLAYERTEST_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UWrapBox* LegsItemsBox;
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UWrapBox* BombsItemsBox;
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UWrapBox* HeartItemsBox;
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UWrapBox* HelmetItemsBox;
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UWrapBox* ChestItemsBox;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UWrapBox* PolearmItems;
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UWrapBox* LongSwordItems;
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UWrapBox* ShieldBoxItems;

	// scroll boxes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* ScrollBoxWeapons;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* ScrollBoxChest;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* ScrollBoxHelmet;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* ScrollBoxLegs;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* ScrollBoxStuff;
protected:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool isArmoryInventory;

public:
	UUserWidget* OwningArmory; // valid only if inside armory

	UInventoryComponent* InventoryComponent;

	void UpdateCorrectInventory(TArray<UItem*>* CurrentItems);

	void UpdateCorrectWrapBox(TEnumAsByte<EItemSubCategory> Category);

	void UpdateCorrectWrapBoxWithEquipChecks(TEnumAsByte<EItemSubCategory> Category, UItemButton* LastClickedButtonInArmory);

	void ShowGivenCategoryInventoryTab(TEnumAsByte<EItemSubCategory> CategoryToShow);

	void ShowGivenCategoryInventoryTabWithOtherSlotEquipChecks(TEnumAsByte<EItemSubCategory> CategoryToShow, UItemButton* LastClickedButtonInArmory);

	UFUNCTION(BlueprintCallable) // set up is happening in HUD blueprints
	void InitializeInventory(UInventoryComponent* newInventoryComponent);
private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UItemsWidget> ItemBlueprintWidget;
	UWrapBox* GetWrapBoxBySubCategory(TEnumAsByte<EItemSubCategory> Category);

	UScrollBox* GetScrollBoxByCategory(TEnumAsByte<EItemSubCategory> Category);

	void UpdateWrapBoxForisEquippedIcon(UWrapBox* WrapBoxToUpdate, UItemButton* LastClickedButtonInArmory);
};
