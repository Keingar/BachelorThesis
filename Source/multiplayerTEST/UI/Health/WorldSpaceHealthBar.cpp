// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldSpaceHealthBar.h"
#include "Components/ProgressBar.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "TimerManager.h"
#include "Components/TextBlock.h"

// Fill out your copyright notice in the Description page of Project Settings.

void UWorldSpaceHealthBar::NativeConstruct()
{
    Super::NativeConstruct();

    HealthBar->SetPercent(1);
    PreviousHealthBar->SetPercent(1);
    HealthText->SetText(FText::FromString(""));

    SetVisibility(ESlateVisibility::Hidden);
}

void UWorldSpaceHealthBar::SetHealth(float CurrentHealth, float MaxHealth)
{
    CalcHealthPercentage(CurrentHealth, MaxHealth);

    SetVisibilityAndText(CurrentHealth, MaxHealth); // I know that it should be the other way around but no cuz previous healht will be fucked up the other way and I don't want to think
}

void UWorldSpaceHealthBar::SetVisibilityAndText(float CurrentHealth, float MaxHealth)
{
    FTimerManager &TimerManager = GetWorld()->GetTimerManager();

    HealthDifference = HealthDifference + (PreviuosHealth - CurrentHealth);

    PreviuosHealth = CurrentHealth;

    HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), HealthDifference)));

    HealthText->SetVisibility(ESlateVisibility::Visible);

    TimerManager.SetTimer(HealthTextVisibilityTimer, this, &UWorldSpaceHealthBar::SetHealthTextNotVisible, 2.f, false);

    changedHealthRecently = true;

    SetDesiredVisibility();

    TimerManager.SetTimer(ChangeHealthVisibilityTimer, this, &UWorldSpaceHealthBar::SetChangeHealthRecentlyFalse, 2.f, false);
}

void UWorldSpaceHealthBar::SetChangeHealthRecentlyFalse()
{
    changedHealthRecently = false;

    SetDesiredVisibility();
}

void UWorldSpaceHealthBar::SetLockedOn(bool bIsLockedOn)
{
    if (!this)
    {
        return;
    }

    if (bIsLockedOn)
    {
        GetWorld()->GetTimerManager().ClearTimer(ChangeLockonVisibilityTimer);

        SetisLockedOnInternal(bIsLockedOn);
    }
    else
    {
        GetWorld()->GetTimerManager().SetTimer(ChangeLockonVisibilityTimer, [this]()
                                               { this->SetisLockedOnInternal(false); }, 2.f, false);
    }
}

void UWorldSpaceHealthBar::SetUpHealthDelegate(UHealthComponent *OwningHealthComponent)
{
    Super::SetUpHealthDelegate(OwningHealthComponent);

    if (OwningHealthComponent)
    {
        PreviuosHealth = OwningHealthComponent->GetCurrentHealth();
    }
}

void UWorldSpaceHealthBar::SetisLockedOnInternal(bool bIsLockedOn)
{
    bisLockedOn = bIsLockedOn;

    SetDesiredVisibility();
}

void UWorldSpaceHealthBar::SetHealthTextNotVisible()
{
    HealthText->SetVisibility(ESlateVisibility::Hidden);
    HealthDifference = 0;
}

void UWorldSpaceHealthBar::SetDesiredVisibility()
{
    if (bisLockedOn || changedHealthRecently)
    {
        SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        SetVisibility(ESlateVisibility::Hidden);
    }
}