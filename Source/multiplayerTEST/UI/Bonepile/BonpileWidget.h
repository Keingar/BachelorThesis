// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/SaveGameSubsystem.h"
#include "BonpileWidget.generated.h"

class UImage;
class UTextBlock;
class UBonepileListWidget;

UCLASS()
class MULTIPLAYERTEST_API UBonpileWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Bonpile")
	void InitializeBonpile(const FBonepileUIEntry& InEntry, bool bAlreadyHere);

	UFUNCTION(BlueprintCallable, Category = "Bonpile")
	void OnChosenBonpile();

	void SetOwningList(UBonepileListWidget* InOwner) { OwningList = InOwner; }

	void UpdateIsAlreadyHere(bool bAlreadyHere);

	const FBonepileUIEntry& GetBonpileEntryData() const { return EntryData; }
protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ScreenshotImage;

	UPROPERTY(meta = (BindWidget))
	UImage* AlreadyHereImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DisplayNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DescriptionText;

	UPROPERTY(BlueprintReadOnly, Category = "Bonpile")
	FBonepileUIEntry EntryData;

	UPROPERTY()
	UBonepileListWidget* OwningList;

	bool bIsAlreadyHere;
};
