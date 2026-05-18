 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemsWidget.generated.h"

class UItem;
class UUserWidget;

UCLASS()
class MULTIPLAYERTEST_API UItemsWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UPROPERTY(BlueprintReadWrite)
	UItem* Item;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void InitiolazeItem();

	UPROPERTY(BlueprintReadOnly)
	UUserWidget* RelatedOwningWidget;

	UFUNCTION(BlueprintCallable)
	void OnItemClicked();

	UPROPERTY(BlueprintReadOnly)
	bool bEquippedNotFromThisSlot;

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateIsFromThisSlotUI(bool FromThisSlot);
};
