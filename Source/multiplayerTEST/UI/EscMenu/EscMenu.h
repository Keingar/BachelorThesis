// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscMenu.generated.h"

class UInventoryWidget;
class UCharacterArmory;

UCLASS()
class MULTIPLAYERTEST_API UEscMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UInventoryWidget* InventoryWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UCharacterArmory* CharacterArmoryWidget;
};
