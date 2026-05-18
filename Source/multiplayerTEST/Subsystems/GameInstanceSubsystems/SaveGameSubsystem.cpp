// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "multiplayerTEST/SaveSystem/CharacterListData.h"
#include "multiplayerTEST/SaveSystem/CharacterData.h"
#include "multiplayerTEST/Items/Weapon.h"
#include "multiplayerTEST/Components/InventoryComponent.h"
#include "Misc/CoreDelegates.h"
#include "multiplayerTEST/CustomStructs/DataTableStructs/BonePileData.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    bSaveDirty = false;
    bSaveInProgress = false;
    AutoSaveInterval = 60.f;

    CharacterListFileName = TEXT("CharacterList");

    CharacterList = LoadCharacterListSafe();
    if (!CharacterList)
    {
        CreateCharacterListSaveFile();
    }

    bShutdownSaveInProgress = false;

#if WITH_EDITOR
    FEditorDelegates::EndPIE.AddUObject(
        this,
        &USaveGameSubsystem::HandleEndPIE
    );
#endif

    TerminateHandle =
        FCoreDelegates::GetApplicationWillTerminateDelegate().AddUObject(
            this,
            &USaveGameSubsystem::HandleTerminate
    );
}

void USaveGameSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
    }

#if WITH_EDITOR
    FEditorDelegates::EndPIE.RemoveAll(this);
#endif

    FCoreDelegates::GetApplicationWillTerminateDelegate().Remove(TerminateHandle);

    Super::Deinitialize();
}

void USaveGameSubsystem::CreateCharacterListSaveFile()
{
    CharacterList = Cast<UCharacterListData>(
        UGameplayStatics::CreateSaveGameObject(UCharacterListData::StaticClass())
    );

    if (!CharacterList)
    {
        return;
    }

    if (!SaveCharacterListSafe())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to persist freshly created CharacterList save file"));
    }
}

void USaveGameSubsystem::CreateNewCharacterSave(FCharacterSaveData NewCharacterData, int ChosenClass)
{
    if (!CharacterList)
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterList is null in CreateNewCharacterSave"));
        return;
    }

    ChosenCharacter = Cast<UCharacterData>(
        UGameplayStatics::CreateSaveGameObject(UCharacterData::StaticClass())
    );

    if (!ChosenCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create CharacterData save object"));
        return;
    }

    FCharacterClassData* CurrentClassData = GetClassFromID(ChosenClass);

    if (!CurrentClassData) return;

    const FString CharacterID = FGuid::NewGuid().ToString(EGuidFormats::Digits);

    NewCharacterData.SavedInventory = CurrentClassData->DefaultInventory;
    NewCharacterData.CharacterID = CharacterID;
    NewCharacterData.OpenedBonepilesIDs.Reset();
    NewCharacterData.bHasSavedTransform = false;
    NewCharacterData.SavedLocation = FVector::ZeroVector;
    NewCharacterData.SavedRotation = FRotator::ZeroRotator;
    NewCharacterData.LastRestedBonpileID = NAME_None;

    ChosenCharacter->SetCharacterData(NewCharacterData);

    ChosenCharacterEntry.CharacterID = CharacterID;

    CharacterList->AddCharacter(CharacterID, NewCharacterData.CharacterName);

    if (!SaveCharacterListSafe())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save CharacterList after adding %s"), *CharacterID);
        return;
    }

    const FString MainSlot = GetMainSlot(CharacterID);
    const FString TempSlot = GetTempSlot(CharacterID);
    const FString BackupSlot = GetBackupSlot(CharacterID);

    if (!UGameplayStatics::SaveGameToSlot(ChosenCharacter, TempSlot, 0))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save new character TEMP slot"));
        return;
    }

    UGameplayStatics::SaveGameToSlot(ChosenCharacter, MainSlot, 0);
    UGameplayStatics::SaveGameToSlot(ChosenCharacter, BackupSlot, 0);
}


void USaveGameSubsystem::LoadChosenCharacter()
{
    const FString CharacterID = ChosenCharacterEntry.CharacterID;
    if (CharacterID.IsEmpty())
    {
        return;
    }

    if (ChosenCharacter &&
        ChosenCharacterEntry.CharacterID == ChosenCharacter->GetCharacterData().CharacterID)
    {
        return;
    }

    UCharacterData* LoadedCharacter = LoadCharacterFromSlot(GetMainSlot(CharacterID));

    if (!LoadedCharacter)
    {
        LoadedCharacter = LoadCharacterFromSlot(GetBackupSlot(CharacterID));
    }

    if (!LoadedCharacter)
    {
        LoadedCharacter = LoadCharacterFromSlot(GetTempSlot(CharacterID));
    }

    if (!LoadedCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load character %s from any slot"), *CharacterID);
        return;
    }

    ChosenCharacter = LoadedCharacter;

    const FCharacterSaveData LoadedData = ChosenCharacter->GetCharacterData();
    OpenedBonepilesIDs = LoadedData.OpenedBonepilesIDs;
}

FCharacterClassData* USaveGameSubsystem::GetClassFromID(int32 ClassID)
{
    if (!ClassesDataTable)
    {
        return nullptr;
    }

    const TArray<FName>& RowNames = ClassesDataTable->GetRowNames();

    if (!RowNames.IsValidIndex(ClassID))
    {
        return nullptr;
    }

    static const FString ContextString(TEXT("GetClassFromID"));

    return ClassesDataTable->FindRow<FCharacterClassData>(
        RowNames[ClassID],
        ContextString
    );
}

bool USaveGameSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return this->GetClass()->IsInBlueprint() && Super::ShouldCreateSubsystem(Outer);
}

bool USaveGameSubsystem::RefreshSavedTransformFromWorld()
{
	if (!ChosenCharacter)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!Pawn)
	{
		return false;
	}

	const FVector CurrLoc = Pawn->GetActorLocation();
	const FRotator CurrRot = Pawn->GetActorRotation();

	FCharacterSaveData Data = ChosenCharacter->GetCharacterData();

	const bool bLocChanged = !Data.SavedLocation.Equals(CurrLoc, 1.0f);        // 1 cm tolerance
	const bool bRotChanged = !Data.SavedRotation.Equals(CurrRot, 0.1f);        // ~0.1° tolerance
	const bool bFirst = !Data.bHasSavedTransform;

	if (!(bFirst || bLocChanged || bRotChanged))
	{
		return false;
	}

	Data.SavedLocation = CurrLoc;
	Data.SavedRotation = CurrRot;
	Data.bHasSavedTransform = true;
	ChosenCharacter->SetCharacterData(Data);
	bSaveDirty = true;
	return true;
}

void USaveGameSubsystem::TryAutoSave()
{
	RefreshSavedTransformFromWorld();

	if (!bSaveDirty ||
		bSaveInProgress ||
		!LocalInventoryComp ||
		!ChosenCharacter ||
		!LocalInventoryComp->GetbIsInventoryInitolized())
	{
		return;
	}

	FCharacterSaveData CurrentCharacterData = ChosenCharacter->GetCharacterData();
	bool bConversionSuccess = true;
	CurrentCharacterData.SavedInventory = ConvertInventoryToSaveData(bConversionSuccess);
    if (!bConversionSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("ForceSave failed to convert inventory for %s"), *CurrentCharacterData.CharacterID);
        return;
    }
	CurrentCharacterData.OpenedBonepilesIDs = OpenedBonepilesIDs;
    CurrentCharacterData.LastVisitedLevel = GetWorld()->GetMapName();

	ChosenCharacter->SetCharacterData(CurrentCharacterData);

	PendingSaveCharacterID = CurrentCharacterData.CharacterID;
	bSaveInProgress = true;
	OnSaveStarted.Broadcast();

	UGameplayStatics::AsyncSaveGameToSlot(
		ChosenCharacter,
		GetTempSlot(PendingSaveCharacterID),
		0,
		FAsyncSaveGameToSlotDelegate::CreateUObject(
			this,
			&USaveGameSubsystem::OnAutoSaveFinished
		)
	);
}

void USaveGameSubsystem::OnAutoSaveFinished(
    const FString& SlotName,
    const int32 UserIndex,
    bool bSuccess
)
{
    bSaveInProgress = false;

    if (!bSuccess)
    {
        PendingSaveCharacterID.Reset();
        OnSaveCompleted.Broadcast();
        UE_LOG(LogTemp, Error, TEXT("Auto-save failed for slot: %s"), *SlotName);
        return;
    }

    const FString CharacterID = PendingSaveCharacterID;
    PendingSaveCharacterID.Reset();

    if (!BackupMainSlot(CharacterID) ||
        !PromoteTempSlot(CharacterID))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to finalize save for %s"), *CharacterID);
    }
    else
    {
        bSaveDirty = false;
    }

    OnSaveCompleted.Broadcast();
}

void USaveGameSubsystem::ForceSave()
{
	RefreshSavedTransformFromWorld();

	if (bSaveInProgress ||
		!LocalInventoryComp ||
		!ChosenCharacter ||
		!LocalInventoryComp->GetbIsInventoryInitolized())
	{
		return;
	}

	FCharacterSaveData CurrentCharacterData = ChosenCharacter->GetCharacterData();
	bool bConversionSuccess = true;
	CurrentCharacterData.SavedInventory = ConvertInventoryToSaveData(bConversionSuccess);
    if(!bConversionSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("ForceSave failed to convert inventory for %s"), *CurrentCharacterData.CharacterID);
        return;
	}
	CurrentCharacterData.OpenedBonepilesIDs = OpenedBonepilesIDs;
    CurrentCharacterData.LastVisitedLevel = GetWorld()->GetMapName();
	ChosenCharacter->SetCharacterData(CurrentCharacterData);

	const FString CharacterID = CurrentCharacterData.CharacterID;
	if (CharacterID.IsEmpty())
	{
		return;
	}

	PendingSaveCharacterID = CharacterID;
	bSaveInProgress = true;
	OnSaveStarted.Broadcast();

	const FString TempSlot = GetTempSlot(PendingSaveCharacterID);
	const bool bSavedTemp = UGameplayStatics::SaveGameToSlot(
		ChosenCharacter,
		TempSlot,
		0
	);

	if (!bSavedTemp)
	{
		PendingSaveCharacterID.Reset();
		bSaveInProgress = false;
		OnSaveCompleted.Broadcast();
		UE_LOG(LogTemp, Error, TEXT("ForceSave failed to write TEMP slot for %s"), *CharacterID);
		return;
	}

	const bool bBackedUp = BackupMainSlot(CharacterID);
	const bool bPromoted = PromoteTempSlot(CharacterID);

	if (!bBackedUp || !bPromoted)
	{
		UE_LOG(LogTemp, Error, TEXT("ForceSave finalize failed for %s (Backup:%s Promote:%s)"),
			*CharacterID,
			bBackedUp ? TEXT("true") : TEXT("false"),
			bPromoted ? TEXT("true") : TEXT("false"));
	}

	if (bBackedUp && bPromoted)
	{
		bSaveDirty = false;
	}

	PendingSaveCharacterID.Reset();
	bSaveInProgress = false;
	OnSaveCompleted.Broadcast();
}

TArray<FItemDataSave> USaveGameSubsystem::ConvertInventoryToSaveData(bool& bIsSuccessfull)
{
    if (!LocalInventoryComp)
    {
        return {};
    }

    TArray<FItemDataSave> SaveDataArray;

    AddArrayToSaveData(LocalInventoryComp->MiscItems, SaveDataArray, bIsSuccessfull);
    if (!bIsSuccessfull) return {};
    AddArrayToSaveData(LocalInventoryComp->HelmetItems, SaveDataArray, bIsSuccessfull);
    if (!bIsSuccessfull) return {};
    AddArrayToSaveData(LocalInventoryComp->ChestItems, SaveDataArray, bIsSuccessfull);
    if (!bIsSuccessfull) return {};
    AddArrayToSaveData(LocalInventoryComp->LegsItems, SaveDataArray, bIsSuccessfull);
    if (!bIsSuccessfull) return {};
    AddArrayToSaveData(LocalInventoryComp->WeaponItems, SaveDataArray, bIsSuccessfull);
    if (!bIsSuccessfull) return {};

    return SaveDataArray;
}

void USaveGameSubsystem::AddArrayToSaveData(
    const TArray<UItem*>& Items,
    TArray<FItemDataSave>& OutSaveData,
    bool& bIsSuccessfull
) const
{
    if (!LocalInventoryComp) return;
    for (UItem* Item : Items)
    {
        const FItemDataSave ItemSaveData = ItemToSaveData(Item, bIsSuccessfull);

        if (!bIsSuccessfull) return;

        if (ItemSaveData.ItemID != 0)
        {
            OutSaveData.Add(ItemSaveData);
        }
    }
}


FItemDataSave USaveGameSubsystem::ItemToSaveData(UItem* Item, bool& bSuccess) const
{
    if (!Item ||
        Item->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
    {
        bSuccess = false;
        return {};
    }
    
    const UWearableItem* WItem = Cast<UWearableItem>(Item);
    if (!IsValid(WItem))
    {
        return {
            Item->Id,
            false,
            0,
            Item->Quantity
        };
    }

    int EquippedSlot = 0;

    if (WItem->bIsEquipped && LocalInventoryComp)
    {
        EquippedSlot = (LocalInventoryComp->SaveWeaponLH1 == Item) ? 2 : 1;
    }
    return {
        WItem->Id,
        WItem->bIsEquipped,
        EquippedSlot,
        WItem->Quantity
    };
}

void USaveGameSubsystem::MarkDirty()
{
    bSaveDirty = true;
}

void USaveGameSubsystem::SetLocalInventoryComponent(UInventoryComponent* InventoryComp)
{
    if (!InventoryComp) return; 

	LocalInventoryComp = InventoryComp;

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    World->GetTimerManager().SetTimer(
        AutoSaveTimerHandle,
        this,
        &USaveGameSubsystem::TryAutoSave,
        AutoSaveInterval,
        true      // looping
    );
}

bool USaveGameSubsystem::BackupMainSlot(const FString& CharacterID) const
{
    const FString MainSlot = GetMainSlot(CharacterID);
    if (!UGameplayStatics::DoesSaveGameExist(MainSlot, 0))
    {
        return true;
    }

    UCharacterData* ExistingMain =
        Cast<UCharacterData>(UGameplayStatics::LoadGameFromSlot(MainSlot, 0));

    if (!ExistingMain)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load MAIN slot for %s"), *CharacterID);
        return false;
    }

    return UGameplayStatics::SaveGameToSlot(
        ExistingMain,
        GetBackupSlot(CharacterID),
        0
    );
}

bool USaveGameSubsystem::PromoteTempSlot(const FString& CharacterID) const
{
    const FString TempSlot = GetTempSlot(CharacterID);
    if (!UGameplayStatics::DoesSaveGameExist(TempSlot, 0))
    {
        UE_LOG(LogTemp, Error, TEXT("TEMP slot missing for %s"), *CharacterID);
        return false;
    }

    UCharacterData* TempData =
        Cast<UCharacterData>(UGameplayStatics::LoadGameFromSlot(TempSlot, 0));

    if (!TempData)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load TEMP slot for %s"), *CharacterID);
        return false;
    }

    const bool bPromoted = UGameplayStatics::SaveGameToSlot(
        TempData,
        GetMainSlot(CharacterID),
        0
    );

    if (bPromoted)
    {
        UGameplayStatics::DeleteGameInSlot(TempSlot, 0);
    }

    return bPromoted;
}

UCharacterData* USaveGameSubsystem::LoadCharacterFromSlot(const FString& SlotName) const
{
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return nullptr;
    }

    return Cast<UCharacterData>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
}

bool USaveGameSubsystem::SaveCharacterListSafe()
{
    if (!CharacterList)
    {
        return false;
    }

    const FString TempSlot = GetCharacterListTempSlot();
    if (!UGameplayStatics::SaveGameToSlot(CharacterList, TempSlot, 0))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to write CharacterList TEMP slot"));
        return false;
    }

    const FString MainSlot = GetCharacterListMainSlot();
    if (UGameplayStatics::DoesSaveGameExist(MainSlot, 0))
    {
        UCharacterListData* ExistingList = LoadCharacterListFromSlot(MainSlot);
        if (ExistingList)
        {
            if (!UGameplayStatics::SaveGameToSlot(ExistingList, GetCharacterListBackupSlot(), 0))
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to back up CharacterList MAIN slot"));
                return false;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Existing CharacterList MAIN slot unreadable; overwriting without backup"));
        }
    }

    const bool bPromoted = UGameplayStatics::SaveGameToSlot(
        CharacterList,
        MainSlot,
        0
    );

    if (!bPromoted)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to promote CharacterList TEMP slot"));
        return false;
    }

    UGameplayStatics::DeleteGameInSlot(TempSlot, 0);
    return true;
}

UCharacterListData* USaveGameSubsystem::LoadCharacterListSafe() const
{
    UCharacterListData* LoadedList = LoadCharacterListFromSlot(GetCharacterListMainSlot());
    if (LoadedList)
    {
        return LoadedList;
    }

    LoadedList = LoadCharacterListFromSlot(GetCharacterListBackupSlot());
    if (LoadedList)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterList MAIN slot missing; loaded BACKUP slot instead"));
        return LoadedList;
    }

    LoadedList = LoadCharacterListFromSlot(GetCharacterListTempSlot());
    if (LoadedList)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterList MAIN and BACKUP missing; loaded TEMP slot"));
    }

    return LoadedList;
}

UCharacterListData* USaveGameSubsystem::LoadCharacterListFromSlot(const FString& SlotName) const
{
    if (SlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return nullptr;
    }

    return Cast<UCharacterListData>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
}

void USaveGameSubsystem::DeleteCharacter(const FCharacterListEntry CharacterToDelete)
{
	if (!CharacterList)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterList is null in DeleteCharacter"));
		return;
	}

	const FString CharacterID = CharacterToDelete.CharacterID;
	if (CharacterID.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("DeleteCharacter called with empty CharacterID"));
		return;
	}

	const FString MainSlot = GetMainSlot(CharacterID);
	const FString TempSlot = GetTempSlot(CharacterID);
	const FString BackupSlot = GetBackupSlot(CharacterID);

	UGameplayStatics::DeleteGameInSlot(MainSlot, 0);
	UGameplayStatics::DeleteGameInSlot(TempSlot, 0);
	UGameplayStatics::DeleteGameInSlot(BackupSlot, 0);

	CharacterList->RemoveCharacterByID(CharacterID);

	if (!SaveCharacterListSafe())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to persist CharacterList after deleting %s"), *CharacterID);
	}

	if (ChosenCharacterEntry.CharacterID == CharacterID)
	{
		ChosenCharacterEntry = {};
		ChosenCharacter = nullptr;
	}
}

void USaveGameSubsystem::PerformShutdownSave()
{
    if (bShutdownSaveInProgress)
        return;

    bShutdownSaveInProgress = true;

    UE_LOG(LogTemp, Log, TEXT("Shutdown Save Triggered (Subsystem)"));

    // Use your existing synchronous ForceSave
    ForceSave();
}

void USaveGameSubsystem::HandleTerminate()
{
    PerformShutdownSave();
}

#if WITH_EDITOR
void USaveGameSubsystem::HandleEndPIE(bool bIsSimulating)
{
    PerformShutdownSave();
}
#endif

bool USaveGameSubsystem::AddOpenedBonpile(const FName BonpileID)
{
    if (BonpileID.IsNone())
    {
        return false;
    }

    const bool bAlreadyOpened = OpenedBonepilesIDs.Contains(BonpileID);
    if (bAlreadyOpened)
    {
        return true;
    }

    OpenedBonepilesIDs.Add(BonpileID);
    bSaveDirty = true;

    return false;
}

bool USaveGameSubsystem::IsBonpileOpened(const FName BonpileID) const
{
    return !BonpileID.IsNone() && OpenedBonepilesIDs.Contains(BonpileID);
}

TArray<FBonepileUIEntry> USaveGameSubsystem::GetOpenedBonpileEntries() const
{
    TArray<FBonepileUIEntry> Result;

    if (!GameBonePiles)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameBonePiles DataTable is null in SaveGameSubsystem"));
        return Result;
    }

    static const FString Context(TEXT("GetOpenedBonpileEntries"));
    for (const FName& BonpileID : OpenedBonepilesIDs)
    {
        if (BonpileID.IsNone())
        {
            continue;
        }

        if (const FBonePileData* Row = GameBonePiles->FindRow<FBonePileData>(BonpileID, Context))
        {
            FBonepileUIEntry Entry;
            Entry.BonpileID = BonpileID;
            Entry.Data = *Row;
            Result.Add(Entry);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Bonpile ID %s not found in GameBonePiles data table"), *BonpileID.ToString());
        }
    }

    return Result;
}

bool USaveGameSubsystem::GetSavedTransform(FVector& OutLocation, FRotator& OutRotation) const
{
	if (!ChosenCharacter)
	{
		return false;
	}

	const FCharacterSaveData Data = ChosenCharacter->GetCharacterData();
	if (!Data.bHasSavedTransform)
	{
		return false;
	}

	OutLocation = Data.SavedLocation;
	OutRotation = Data.SavedRotation;
	return true;
}

void USaveGameSubsystem::SetLastRestedBonpile(FName BonpileID)
{
	if (!ChosenCharacter || BonpileID.IsNone())
	{
		return;
	}

	FCharacterSaveData Data = ChosenCharacter->GetCharacterData();
	if (Data.LastRestedBonpileID == BonpileID)
	{
		return;
	}

	Data.LastRestedBonpileID = BonpileID;
	ChosenCharacter->SetCharacterData(Data);
	bSaveDirty = true;
}

bool USaveGameSubsystem::GetLastRestedBonpileID(FName& OutBonpileID) const
{
	if (!ChosenCharacter)
	{
		return false;
	}

	const FCharacterSaveData Data = ChosenCharacter->GetCharacterData();
	if (Data.LastRestedBonpileID.IsNone())
	{
		return false;
	}

	OutBonpileID = Data.LastRestedBonpileID;
	return true;
}

bool USaveGameSubsystem::isSavedTransformFromCurrentLevel()
{
    if (!ChosenCharacter) return false;
    
    return ChosenCharacter->GetCharacterData().LastVisitedLevel == GetWorld()->GetMapName();
}

void USaveGameSubsystem::MarkEnemyDead(FName EnemyID, bool isOneTimeEnemy)
{
	if (!ChosenCharacter || EnemyID.IsNone())
	{
		return;
	}

	FCharacterSaveData Data = ChosenCharacter->GetCharacterData();

    // if true then boss if not regular enemy
    if (isOneTimeEnemy) {
        if(Data.DeadOneTimeEnemiesIDs.Contains(EnemyID)) return;

        Data.DeadOneTimeEnemiesIDs.Add(EnemyID);
    }
    else {
        if (Data.DeadEnemyIDs.Contains(EnemyID)) return;

        Data.DeadEnemyIDs.Add(EnemyID);
    }

	ChosenCharacter->SetCharacterData(Data);
	bSaveDirty = true;
}

bool USaveGameSubsystem::IsEnemyDead(FName EnemyID, bool isOneTimeEnemy) const
{
	if (!ChosenCharacter || EnemyID.IsNone())
	{
		return false;
	}

    const FCharacterSaveData Data = ChosenCharacter->GetCharacterData();

    if (isOneTimeEnemy) {
        return Data.DeadOneTimeEnemiesIDs.Contains(EnemyID);
    }

	return Data.DeadEnemyIDs.Contains(EnemyID);
}

void USaveGameSubsystem::ClearDeadEnemies()
{
	if (!ChosenCharacter)
	{
		return;
	}

	FCharacterSaveData Data = ChosenCharacter->GetCharacterData();
	if (Data.DeadEnemyIDs.Num() == 0)
	{
		return;
	}

	Data.DeadEnemyIDs.Reset();
	Data.bHasSavedTransform = false; // optional: clear cached transform on full reset
	ChosenCharacter->SetCharacterData(Data);
	bSaveDirty = true;
}

#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG

void USaveGameSubsystem::ResetOneTimeEnemies()
{
    if (!ChosenCharacter)
    {
        return;
    }

    FCharacterSaveData Data = ChosenCharacter->GetCharacterData();
    if (Data.DeadOneTimeEnemiesIDs.Num() == 0)
    {
        return;
    }

    Data.DeadOneTimeEnemiesIDs.Reset();
    Data.bHasSavedTransform = false;
    ChosenCharacter->SetCharacterData(Data);
    bSaveDirty = true;
}

#endif