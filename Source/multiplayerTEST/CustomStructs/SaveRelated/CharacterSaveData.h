#pragma once

#include "CoreMinimal.h"
#include "multiplayerTEST/CustomStructs/SaveRelated/ItemDataSave.h"
#include "CharacterSaveData.generated.h"

USTRUCT(BlueprintType)
struct FCharacterSaveData
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    FText CharacterName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    int ChosenMaterialID;
    UPROPERTY(SaveGame)
    TArray<FItemDataSave> SavedInventory;

    UPROPERTY(SaveGame)
    FString CharacterID;

    UPROPERTY(SaveGame)
    TArray<FName> OpenedBonepilesIDs;

    UPROPERTY(SaveGame)
    bool bHasSavedTransform = false;

    UPROPERTY(SaveGame)
    FVector SavedLocation = FVector::ZeroVector;

    UPROPERTY(SaveGame)
    FRotator SavedRotation = FRotator::ZeroRotator;

    UPROPERTY(SaveGame)
    FName LastRestedBonpileID = NAME_None;

    UPROPERTY(SaveGame)
    FString LastVisitedLevel;

    UPROPERTY(SaveGame)
    TArray<FName> DeadEnemyIDs;   // registry IDs of defeated enemies

    UPROPERTY(SaveGame)
    TArray<FName> DeadOneTimeEnemiesIDs; // for bosses or elites
};
