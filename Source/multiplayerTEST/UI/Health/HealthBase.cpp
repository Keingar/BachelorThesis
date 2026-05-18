// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthBase.h"
#include "Components/ProgressBar.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "TimerManager.h"
#include "Components/TextBlock.h"

// Fill out your copyright notice in the Description page of Project Settings.

void UHealthBase::NativeConstruct()
{
    Super::NativeConstruct();

    HealthBar->SetPercent(1);
    PreviousHealthBar->SetPercent(1);
}

void UHealthBase::SetHealth(float CurrentHealth, float MaxHealth)
{
    CalcHealthPercentage(CurrentHealth, MaxHealth);
}

void UHealthBase::CalcHealthPercentage(float CurrentHealth, float MaxHealth)
{
    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    if (HealthBar) {
        HealthBar->SetPercent(CurrentHealth / MaxHealth);
    }

    if (!TimerManager.IsTimerActive(PreviousHealthbarMachingTimer))
    {
        TimerManager.SetTimer(PreviousHealthbarMachingTimer, this, &UHealthBase::UpdatePreviousHealtBar, 0.025f, true, 0.6f);
    }
}

void UHealthBase::SetUpHealthDelegate(UHealthComponent* OwningHealthComponent)
{
	OwnerHealthComponent = OwningHealthComponent;
   // SetUpHealthStuff();
    GetWorld()->GetTimerManager().SetTimer(HandleToSetuPHealthStuff, this, &UHealthBase::SetUpHealthStuff, 0.1, false, 0.1);

}

void UHealthBase::SetUpHealthStuff()
{
    if (!OwnerHealthComponent) {
        return;
    }

    if (OwnerHealthComponent->GetCurrentHealth() <= 0)
    { // sometimes this set up faster then health component set up magically (which makes literally -1 sense)
        GetWorld()->GetTimerManager().SetTimer(HandleToSetuPHealthStuff, this, &UHealthBase::SetUpHealthStuff, 0.1, false, 0.1);

    }
    GetWorld()->GetTimerManager().ClearTimer(HandleToSetuPHealthStuff);
    OwnerHealthComponent->OnHealthChangedLeftOver.AddUObject(this, &UHealthBase::SetHealth);
    HealthBar->SetPercent(OwnerHealthComponent->GetCurrentHealth() / OwnerHealthComponent->GetMaxHealth());
    PreviousHealthBar->SetPercent(HealthBar->GetPercent());
}

void UHealthBase::UpdatePreviousHealtBar()
{
    PreviousHealthBar->SetPercent(PreviousHealthBar->GetPercent() - 0.018f);

    if (PreviousHealthBar->GetPercent() < HealthBar->GetPercent()) {
        PreviousHealthBar->SetPercent(HealthBar->GetPercent());

        GetWorld()->GetTimerManager().ClearTimer(PreviousHealthbarMachingTimer);
    }
}