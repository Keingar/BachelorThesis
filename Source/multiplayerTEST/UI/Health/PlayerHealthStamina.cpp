// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHealthStamina.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "multiplayerTEST/Characters/multiplayerTESTCharacter.h"
#include "MultiplayerTEST/Components/HealthComponent.h"
#include "HealthBase.h"
#include "MultiplayerTEST/Components/StaminaComponent.h"


void UPlayerHealthStamina::NativeConstruct()
{
    Super::NativeConstruct();

    StaminaBar->SetPercent(1);
}

void UPlayerHealthStamina::UpdateStaminaBar(float newPercent)
{
    if (StaminaBar) {
        StaminaBar->SetPercent(newPercent);
    }
}

void UPlayerHealthStamina::CalcHealthPercentage(float CurrentHealth, float MaxHealth)
{
	Super::CalcHealthPercentage(CurrentHealth, MaxHealth);
    
	ChangeSkullIcon(CurrentHealth / MaxHealth * 100);
}

void UPlayerHealthStamina::SetUpHealthDelegateTimer()
{
    if (!GetOwningPlayerPawn()) return;

    if (UHealthComponent* HealthComponent = GetOwningPlayerPawn()->GetComponentByClass<UHealthComponent>()) {
        SetUpHealthDelegate(HealthComponent);
    }
    else {
        FTimerHandle TimerHandle_SetUpDelegate;

        GetWorld()->GetTimerManager().SetTimer(TimerHandle_SetUpDelegate, this, &UPlayerHealthStamina::SetUpHealthDelegateTimer, 0.1f, false);
    }
}

void UPlayerHealthStamina::SetUpStaminaDelegate()
{
    StaminaBar->SetPercent(1);

    if (!GetOwningPlayerPawn()) return;

    if (UStaminaComponent* StaminaComponent = GetOwningPlayerPawn()->GetComponentByClass<UStaminaComponent>()) {
        StaminaComponent->OnStaminaUpdate.BindUObject(this, &UPlayerHealthStamina::UpdateStaminaBar);
    }
    else {
        FTimerHandle TimerHandle_SetUpDelegate;

        GetWorld()->GetTimerManager().SetTimer(TimerHandle_SetUpDelegate, this, &UPlayerHealthStamina::SetUpStaminaDelegate, 0.1f, false);
    }
}
