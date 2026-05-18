// Fill out your copyright notice in the Description page of Project Settings.


#include "OtherPlayerHealthHudUI.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

// get info to remove later	
// set up nickname wihtout obstacle check

void UOtherPlayerHealthHudUI::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Visible);
	NicknameText->SetVisibility(ESlateVisibility::Visible);
	HealthBar->SetVisibility(ESlateVisibility::Visible);
	PreviousHealthBar->SetVisibility(ESlateVisibility::Visible);
}

void UOtherPlayerHealthHudUI::OnSomeoneDeath(AActor* DeadActor)
{
}

void UOtherPlayerHealthHudUI::SetDesiredVisibility()
{

}

void UOtherPlayerHealthHudUI::SetUpOnSomeoneDeath(UHealthComponent* OwningHealthComponent)
{
}
