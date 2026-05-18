// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDataSave.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FItemDataSave
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    int ItemID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    bool bIsEquipped;

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        SaveGame,
        meta=(ToolTip="Used only for weapons to determine the exact equipped slot, now 1 = right hand and 2 = left hand")
    )
    int EquippedSlot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    int Quantity;
};   