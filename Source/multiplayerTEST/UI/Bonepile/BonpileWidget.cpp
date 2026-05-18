// Fill out your copyright notice in the Description page of Project Settings.

#include "BonpileWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "multiplayerTEST/UI/Bonepile/BonepileListWidget.h"

void UBonpileWidget::InitializeBonpile(const FBonepileUIEntry& InEntry, bool bAlreadyHere)
{
	EntryData = InEntry;
	bIsAlreadyHere = bAlreadyHere;

	if (DisplayNameText)
	{
		DisplayNameText->SetText(EntryData.Data.DisplayName);
	}

	if (DescriptionText)
	{
		DescriptionText->SetAutoWrapText(true);
		DescriptionText->SetWrapTextAt(0.f);
		DescriptionText->SetText(EntryData.Data.Description);
	}

	if (ScreenshotImage)
	{
		UTexture2D* Texture = EntryData.Data.MainScreenshot.IsValid()
			? EntryData.Data.MainScreenshot.Get()
			: EntryData.Data.MainScreenshot.LoadSynchronous();

		if (Texture)
		{
			ScreenshotImage->SetBrushFromTexture(Texture);
		}

		FSlateBrush Brush = ScreenshotImage->GetBrush();
		Brush.ImageSize = FVector2D(250.f, 250.f);
		ScreenshotImage->SetBrush(Brush);
	}

	UpdateIsAlreadyHere(bIsAlreadyHere);
}

void UBonpileWidget::OnChosenBonpile()
{
	if (OwningList && !EntryData.BonpileID.IsNone())
	{
		OwningList->HandleBonpileChosen(EntryData.BonpileID, bIsAlreadyHere);
	}
}

void UBonpileWidget::UpdateIsAlreadyHere(bool bAlreadyHere)
{
	bIsAlreadyHere = bAlreadyHere; 

	if (!AlreadyHereImage) return;

	if (bIsAlreadyHere) {
		AlreadyHereImage->SetVisibility(ESlateVisibility::Visible);
	}
	else {
		AlreadyHereImage->SetVisibility(ESlateVisibility::Hidden);
	}
}
