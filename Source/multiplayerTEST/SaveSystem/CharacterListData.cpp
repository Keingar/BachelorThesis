// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterListData.h"

void UCharacterListData::AddCharacter(const FString CharacterID, const FText DisplayName)
{
    FCharacterListEntry NewEntry;
    NewEntry.CharacterID = CharacterID;
    NewEntry.DisplayName = DisplayName;

    CharacterList.Add(NewEntry);
}

void UCharacterListData::RemoveCharacterByID(const FString CharacterID)
{
    CharacterList.RemoveAll(
        [&](const FCharacterListEntry& Entry)
        {
            return Entry.CharacterID == CharacterID;
        }
    );
}
