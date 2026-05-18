// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "multiplayerTEST/CustomStructs/SaveRelated/CharacterListEntry.h"
#include "CharacterListData.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UCharacterListData : public USaveGame
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure = false)
    TArray<FCharacterListEntry> GetCharacterList() const
    {
        return CharacterList;
    }

    void AddCharacter(const FString CharacterID, const FText DisplayName);

    void RemoveCharacterByID(const FString CharacterID);

    bool isValidFile; 
protected:
    UPROPERTY(SaveGame)
    TArray<FCharacterListEntry> CharacterList;
};
