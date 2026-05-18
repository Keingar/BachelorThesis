// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSubsystem.h"
#include "multiplayerTEST/Interactable/PickUpActorCore.h"
#include "multiplayerTEST/Items/WearableItem.h"

UItem* UItemSubsystem::GetItemInstanceFromID(int ItemID, UObject* Outer)
{
    if (!this || !Outer || ItemID == 0 || !ItemDataTable)
    {
        return nullptr; // no log error here because ItemID will be zero when trying to take off something or in set up func
    }

    const FItemDataForTable* ItemData = GetItemDataFromID(ItemID);

    if (!ItemData || ItemData->ItemClass.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("ItemData is nullptr or ItemClass path is null"));
        return nullptr;
    }

    // Load the soft class reference
    UClass* ItemClass = ItemData->ItemClass.Get();
    if (!ItemClass)
    {
        ItemClass = ItemData->ItemClass.LoadSynchronous();
    }

    if (!ItemClass) {
        UE_LOG(LogTemp, Error, TEXT("ItemClass is nullptr inside GetItemInstanceFromID"));
        return nullptr;
    }

    return NewObject<UItem>(Outer, ItemClass);
}

void UItemSubsystem::GetInventoryFromSavedItemData(
    const TArray<FItemDataSave>& SavedInventory,
    UObject* Outer,
    TArray<UItem*>& OutInventory
)
{
    OutInventory.Reset();

    if (!Outer || !ItemDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("GetInventoryFromSavedItemData failed: Outer or ItemDataTable is null"));
        return;
    }

    for (const FItemDataSave& SavedItem : SavedInventory)
    {
        if (SavedItem.ItemID == 0)
        {
            continue;
        }

        UItem* ItemInstance = GetItemInstanceFromID(SavedItem.ItemID, Outer);
        if (!ItemInstance)
        {
            UE_LOG(LogTemp, Warning, TEXT("Could not create item instance for ID %d"), SavedItem.ItemID);
            continue;
        }

        ItemInstance->Quantity = SavedItem.Quantity;

        UWearableItem* WearableItem = Cast<UWearableItem>(ItemInstance);
        if (WearableItem)
        {
            WearableItem->bIsEquipped = SavedItem.bIsEquipped;

            WearableItem->EquippedSlot = SavedItem.EquippedSlot;
        }

        OutInventory.Add(ItemInstance);
    }
}

UItem* UItemSubsystem::GetItemInstanceFromItemData(const FItemDataForTable* ItemData, UObject* Outer)
{
    if (!this || !Outer) {
        UE_LOG(LogTemp, Error, TEXT("ItemData is nullptr is invalid inside GetItemInstanceFromID"));
        return nullptr;
    }

    if (!ItemData || ItemData->ItemClass.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("ItemData is nullptr or ItemClass path is null"));
        return nullptr;
    }

    // Load the soft class reference
    UClass* ItemClass = ItemData->ItemClass.Get();
    if (!ItemClass)
    {
        ItemClass = ItemData->ItemClass.LoadSynchronous();
    }

    if (!ItemClass) {
        UE_LOG(LogTemp, Error, TEXT("ItemClass is nullptr inside GetItemInstanceFromID"));
        return nullptr;
    }

    return NewObject<UItem>(Outer, ItemClass);
}

FItemDataForTable* UItemSubsystem::GetItemDataFromID(int ItemID)
{
    return ItemDataTable->FindRow<FItemDataForTable>(FName(*FString::FromInt(ItemID)), TEXT("Lookup Item"));
}

void UItemSubsystem::SpawnDroppedItemsAtLocation(const TArray<int>& ItemIDs, const FVector& Location, bool bSaveOnReset)
{
    if (ItemIDs.IsEmpty() || !ItemDataTable || !PickupActorClass) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    TArray<FItemDataForTable*> LootItemData;

    for (int32 ItemID : ItemIDs)
    {
        FItemDataForTable* ItemData = GetItemDataFromID(ItemID);

        if (ItemData)
        {
            LootItemData.Add(ItemData);  // Store found item data
        }
    }

    APickUpActorCore* SpawnedPickup = World->SpawnActor<APickUpActorCore>(PickupActorClass, Location, FRotator::ZeroRotator, SpawnParams);

    if (SpawnedPickup)
    {
        SpawnedPickup->SetLoot(LootItemData);
        SpawnedPickup->SaveOnReset = bSaveOnReset;
    }
}

bool UItemSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return this->GetClass()->IsInBlueprint() && Super::ShouldCreateSubsystem(Outer);
}
