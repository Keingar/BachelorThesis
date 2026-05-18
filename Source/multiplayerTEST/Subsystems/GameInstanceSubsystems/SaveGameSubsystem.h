// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "multiplayerTEST/CustomStructs/SaveRelated/CharacterListEntry.h"
#include "multiplayerTEST/CustomStructs/DataTableStructs/CharacterClassData.h"
#include "multiplayerTEST/CustomStructs/SaveRelated/CharacterSaveData.h"
#include "multiplayerTEST/CustomStructs/DataTableStructs/BonePileData.h"
#include "SaveGameSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FBonepileUIEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FName BonpileID;

	UPROPERTY(BlueprintReadOnly)
	FBonePileData Data;
};

/**
 * 
 */
class UInventoryComponent;
class UCharacterData;
class UCharacterListData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveStatusChanged);

UCLASS(BlueprintType, Blueprintable)
class MULTIPLAYERTEST_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;
public:

	UFUNCTION(BlueprintCallable)
	UCharacterListData* GetCharacterListSaveObject() const { return CharacterList; }
	UFUNCTION(BlueprintCallable)
	UCharacterData* GetChosenCharacter() const { return ChosenCharacter; }
	FCharacterListEntry GetChosenCharacterEntry() const { return ChosenCharacterEntry; }

	UFUNCTION(BlueprintCallable)
	void SetChosenCharacterEntry(const FCharacterListEntry NewChosenCharacterEntry) { ChosenCharacterEntry = NewChosenCharacterEntry; }

	UFUNCTION(BlueprintCallable)
	void CreateNewCharacterSave(FCharacterSaveData NewCharacterData, int ChosenClass);

	UFUNCTION(BlueprintCallable)
	void DeleteCharacter(const FCharacterListEntry CharacterToDelete);

	UFUNCTION(BlueprintCallable)
	void LoadChosenCharacter();
	UFUNCTION(BlueprintCallable)
	UDataTable* GetClassesDataTable() { return ClassesDataTable; }

	UFUNCTION(BlueprintCallable)
	UDataTable* GetPlayerSkinsDataTable() { return PlayerSkinsDataTable; }

	UFUNCTION(BlueprintCallable)
	void MarkDirty();

	void SetLocalInventoryComponent(UInventoryComponent* InventoryComp);

	UFUNCTION(BlueprintCallable)
	void ForceSave();

	UFUNCTION(BlueprintCallable)
	bool AddOpenedBonpile(const FName BonpileID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsBonpileOpened(const FName BonpileID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FBonepileUIEntry> GetOpenedBonpileEntries() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FName> GetOpenedBonpiles() const { return OpenedBonepilesIDs; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetSavedTransform(FVector& OutLocation, FRotator& OutRotation) const;

	UFUNCTION(BlueprintCallable)
	void SetLastRestedBonpile(FName BonpileID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetLastRestedBonpileID(FName& OutBonpileID) const;

	bool isSavedTransformFromCurrentLevel();

	UPROPERTY(BlueprintAssignable)
	FOnSaveStatusChanged OnSaveStarted;
	UPROPERTY(BlueprintAssignable)
	FOnSaveStatusChanged OnSaveCompleted;


#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
	UDataTable* GetGameBonePilesDataTable() const { return GameBonePiles; }

	void ResetOneTimeEnemies();
#endif


protected:
	UPROPERTY()
	UCharacterData* ChosenCharacter;
	UPROPERTY()
	UCharacterListData* CharacterList;

	FString CharacterListFileName;

	// used to load character not when chosing but when starting to load level after that this variable is obsolete
	FCharacterListEntry ChosenCharacterEntry;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* ClassesDataTable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* PlayerSkinsDataTable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* GameBonePiles;
	UPROPERTY()
	TArray<FName> OpenedBonepilesIDs;

	bool bSaveDirty;

	UInventoryComponent* LocalInventoryComp;
	

	FTimerHandle AutoSaveTimerHandle;
	float AutoSaveInterval;

	bool bSaveInProgress;
	FString PendingSaveCharacterID;

protected:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	FCharacterClassData* GetClassFromID(int ClassID);

	void CreateCharacterListSaveFile();
	bool SaveCharacterListSafe();
	UCharacterListData* LoadCharacterListSafe() const;
	UCharacterListData* LoadCharacterListFromSlot(const FString& SlotName) const;

	void TryAutoSave();
	void OnAutoSaveFinished(const FString& SlotName, const int32 UserIndex, bool bSuccess);

	bool BackupMainSlot(const FString& CharacterID) const;
	bool PromoteTempSlot(const FString& CharacterID) const;
	UCharacterData* LoadCharacterFromSlot(const FString& SlotName) const;

	TArray<FItemDataSave> ConvertInventoryToSaveData(bool& bIsSuccessfull);
	void AddArrayToSaveData(const TArray<class UItem*>& Items, TArray<FItemDataSave>& OutSaveData, bool& bIsSuccessfull) const;

	FItemDataSave ItemToSaveData(class UItem* Item, bool& bSuccess) const;

	FString GetMainSlot(const FString& ID) const { return ID + TEXT("_MAIN"); }
	FString GetTempSlot(const FString& ID) const { return ID + TEXT("_TMP"); }
	FString GetBackupSlot(const FString& ID) const { return ID + TEXT("_BAK"); }

	FString GetCharacterListMainSlot() const { return CharacterListFileName; }
	FString GetCharacterListTempSlot() const { return CharacterListFileName + TEXT("_TMP"); }
	FString GetCharacterListBackupSlot() const { return CharacterListFileName + TEXT("_BAK"); }

public:
	UFUNCTION(BlueprintCallable)
	void MarkEnemyDead(FName EnemyID, bool isOneTimeEnemy);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsEnemyDead(FName EnemyID, bool isOneTimeEnemy) const;

	UFUNCTION(BlueprintCallable)
	void ClearDeadEnemies();

private:
	bool RefreshSavedTransformFromWorld();

	void PerformShutdownSave();
	void HandleTerminate();
	
#if WITH_EDITOR
	void HandleEndPIE(bool bIsSimulating);
#endif

	FDelegateHandle TerminateHandle;
	bool bShutdownSaveInProgress = false;

};
