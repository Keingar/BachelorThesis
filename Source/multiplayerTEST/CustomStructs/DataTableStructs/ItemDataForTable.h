// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "multiplayerTEST/Items/Item.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPtr.h"
#include "ItemDataForTable.generated.h"


USTRUCT(BlueprintType)
struct FItemDataForTable : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UItem> ItemClass;

};
