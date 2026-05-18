// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BonepileListWidget.generated.h"

class UWrapBox;
class UBonpileWidget;
class AmultiplayerTESTGameMode;
class USaveGameSubsystem;

UCLASS()
class MULTIPLAYERTEST_API UBonepileListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Bonpile")
	void RefreshBonpileList();

	UFUNCTION(BlueprintCallable, Category = "Bonpile")
	void HandleBonpileChosen(FName BonpileID, bool bAlreadyHere);

	UFUNCTION(BlueprintCallable)
	void SetCurrentBonepileID(FName NewBonepileID) {
		CurrentBonepileID = NewBonepileID;
	}

protected:
	UPROPERTY(meta = (BindWidget))
	UWrapBox* BonpileWrapBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bonpile")
	TSubclassOf<UBonpileWidget> BonpileWidgetClass;

	FName CurrentBonepileID;

	USaveGameSubsystem* SaveSubsystem;

	AmultiplayerTESTGameMode* CachedGM;

	TArray<UBonpileWidget*> BonpileWidgets;
};
