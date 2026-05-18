// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "multiplayerTEST/CustomStructs/SaveRelated/ItemDataSave.h"
#include "CharacterClassData.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FCharacterClassData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FItemDataSave> DefaultInventory;
};