// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "multiplayerTEST/CustomStructs/DataTableStructs/ItemDataForTable.h"
#include "multiplayerTEST/CustomStructs/SaveRelated/ItemDataSave.h"
#include "ItemSubsystem.generated.h"

class UItem;
class APickUpActorCore;

UCLASS(BlueprintType, Blueprintable)
class MULTIPLAYERTEST_API UItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FItemDataForTable* GetItemDataFromID(int ItemID);

	UItem* GetItemInstanceFromID(int ItemID, UObject* Outer);

	UFUNCTION(BlueprintCallable)
	void GetInventoryFromSavedItemData(
		const TArray<FItemDataSave>& SavedInventory,
		UObject* Outer,
		TArray<UItem*>& OutInventory
	);

	UItem* GetItemInstanceFromItemData(const FItemDataForTable* ItemData, UObject* Outer);

	void SpawnDroppedItemsAtLocation(const TArray<int>& ItemIDs, const FVector& Location, bool bSaveOnReset);

	const UDataTable* GetItemDataTable() const { return ItemDataTable; }

protected:

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* ItemDataTable;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APickUpActorCore> PickupActorClass;
};
