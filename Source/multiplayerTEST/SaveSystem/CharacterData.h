// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "multiplayerTEST/CustomStructs/SaveRelated/CharacterSaveData.h"
#include "CharacterData.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UCharacterData : public USaveGame
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SaveSystem")
    FCharacterSaveData GetCharacterData() const { return CharacterData; }

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
    void SetCharacterData(const FCharacterSaveData& NewCharacterData);

    bool isValidFile;
protected:
    UPROPERTY(SaveGame)
    FCharacterSaveData CharacterData;
};
