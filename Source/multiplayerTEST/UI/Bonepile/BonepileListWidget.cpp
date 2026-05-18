// Fill out your copyright notice in the Description page of Project Settings.


#include "BonepileListWidget.h"
#include "Components/WrapBox.h"
#include "BonpileWidget.h"
#include "Engine/GameInstance.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/SaveGameSubsystem.h"
#include "multiplayerTEST/GameModeRelated/multiplayerTESTGameMode.h"

void UBonepileListWidget::NativeConstruct()
{
	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance) {
		SaveSubsystem = GameInstance->GetSubsystem<USaveGameSubsystem>();
	}
	
	UWorld* World = GetWorld();
	if (!World) return;

	CachedGM = World->GetAuthGameMode<AmultiplayerTESTGameMode>();

}

void UBonepileListWidget::RefreshBonpileList()
{
	if (!BonpileWrapBox || !BonpileWidgetClass)
	{
		return;
	}

	BonpileWrapBox->ClearChildren();
	BonpileWidgets.Empty();

	if (!SaveSubsystem) return;
	
	const TArray<FBonepileUIEntry> Entries = SaveSubsystem->GetOpenedBonpileEntries();

	for (const FBonepileUIEntry& Entry : Entries)
	{
		UBonpileWidget* Widget = CreateWidget<UBonpileWidget>(this, BonpileWidgetClass);
		if (!Widget)
		{
			continue;
		}

		bool bIsAlreadyHere = Entry.BonpileID == CurrentBonepileID;

		Widget->SetOwningList(this);
		Widget->InitializeBonpile(Entry, bIsAlreadyHere);
		BonpileWrapBox->AddChildToWrapBox(Widget);
		BonpileWidgets.Add(Widget);
	}
}

void UBonepileListWidget::HandleBonpileChosen(FName BonpileID, bool bAlreadyHere)
{
	if (BonpileID.IsNone() || !CachedGM || bAlreadyHere)
	{
		return;
	}
	
	CachedGM->TeleportAllPlayersToBonpile(BonpileID);
	
	CurrentBonepileID = BonpileID;

	for (UBonpileWidget* Widget : BonpileWidgets)
	{
		const bool bIsNowHere = Widget->GetBonpileEntryData().BonpileID == BonpileID;
		Widget->UpdateIsAlreadyHere(bIsNowHere);
	}
}

