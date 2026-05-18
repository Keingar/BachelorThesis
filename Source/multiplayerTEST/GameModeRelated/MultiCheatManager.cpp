// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiCheatManager.h"
#include "GameFramework/PlayerController.h"
#include "multiplayerTEST/Components/InventoryComponent.h"
#include "GameFramework/PlayerState.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/ItemSubsystem.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/SaveGameSubsystem.h"

void UMultiCheatManager::GiveAllItems()
{
    APlayerController* PC = GetOuterAPlayerController();
    if (!PC) return;

    APlayerState* PS = PC->GetPlayerState<APlayerState>();
    if (!PS) return;

    UInventoryComponent* Inventory = PS->FindComponentByClass<UInventoryComponent>();
    if (!Inventory) return;

    UGameInstance* GI = PC->GetGameInstance();
    if (!GI) return;

    UItemSubsystem* ItemSubsystem = GI->GetSubsystem<UItemSubsystem>();
    if (!ItemSubsystem) return;

    const UDataTable* ItemDataTable = ItemSubsystem->GetItemDataTable();
    if (!ItemDataTable) return;

    for (const FName& RowName : ItemDataTable->GetRowNames())
    {
        FItemDataForTable* ItemData = ItemDataTable->FindRow<FItemDataForTable>(RowName, TEXT("GiveAllItems"));
        if (!ItemData) continue;
        
        UItem* NewItem = ItemSubsystem->GetItemInstanceFromItemData(ItemData, Inventory);

        if (!NewItem) {
            UE_LOG(LogTemp, Error, TEXT("Wasnt able to get item from the table when inside GiveAllItems"));
            continue;
        }

        Inventory->AddItem(NewItem);

        // give second time if weapon
        if (NewItem->ItemSubCategory == EItemSubCategory::LongSword ||
            NewItem->ItemSubCategory == EItemSubCategory::Polearm)
        {
            UItem* SecondItem = ItemSubsystem->GetItemInstanceFromItemData(ItemData, Inventory);

            if (SecondItem)
            {
                Inventory->AddItem(SecondItem);
            }
        }
	}

    UE_LOG(LogTemp, Warning, TEXT("Cheat: GiveAllItems executed"));
}

void UMultiCheatManager::GetItemByID(int ItemID)
{
    APlayerController* PC = GetOuterAPlayerController();
    if (!PC) return;

    APlayerState* PS = PC->GetPlayerState<APlayerState>();
    if (!PS) return;

    UInventoryComponent* Inventory = PS->FindComponentByClass<UInventoryComponent>();
    if (!Inventory) return;

    UGameInstance* GI = PC->GetGameInstance();
    if (!GI) return;

    UItemSubsystem* ItemSubsystem = GI->GetSubsystem<UItemSubsystem>();
    if (!ItemSubsystem) return;

    UItem* NewItem = ItemSubsystem->GetItemInstanceFromID(ItemID, Inventory);

    if (NewItem)
    {
        Inventory->AddItem(NewItem);
    }

    UE_LOG(LogTemp, Warning, TEXT("Cheat: Added Item ID %d"), ItemID);
}

void UMultiCheatManager::OpenAllBonepiles()
{
    APlayerController* PC = GetOuterAPlayerController();
    if (!PC) return;

    APlayerState* PS = PC->GetPlayerState<APlayerState>();
    if (!PS) return;

    UInventoryComponent* Inventory = PS->FindComponentByClass<UInventoryComponent>();
    if (!Inventory) return;

    UGameInstance* GI = PC->GetGameInstance();
    if (!GI) return;

    USaveGameSubsystem* SaveSubsystem = GI->GetSubsystem<USaveGameSubsystem>();
    if (!SaveSubsystem) return;

    UDataTable* BonepileTable = SaveSubsystem->GetGameBonePilesDataTable();
    if (!BonepileTable) return;

    const FString ContextString(TEXT("OpenAllBonepiles"));

    TArray<FName> RowNames = BonepileTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        SaveSubsystem->AddOpenedBonpile(RowName);

        UE_LOG(LogTemp, Warning, TEXT("Opened Bonepile: %s"), *RowName.ToString());
    }
}

void UMultiCheatManager::ResetBossess()
{
    APlayerController* PC = GetOuterAPlayerController();
    if (!PC) return;

    APlayerState* PS = PC->GetPlayerState<APlayerState>();
    if (!PS) return;

    UInventoryComponent* Inventory = PS->FindComponentByClass<UInventoryComponent>();
    if (!Inventory) return;

    UGameInstance* GI = PC->GetGameInstance();
    if (!GI) return;

    USaveGameSubsystem* SaveSubsystem = GI->GetSubsystem<USaveGameSubsystem>();
    if (!SaveSubsystem) return;

	SaveSubsystem->ResetOneTimeEnemies();
}
