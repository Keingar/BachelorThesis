// Fill out your copyright notice in the Description page of Project Settings.


#include "StaminaComponent.h"
#include "Net/UnrealNetwork.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/TagManagerOwner.h"
#include "multiplayerTEST/Components/HealthComponent.h"

// Sets default values for this component's properties
UStaminaComponent::UStaminaComponent()
{

    PrimaryComponentTick.bCanEverTick = false;

    // Default values (you can override these in the editor)
    MaxStamina = 100.0f;
    CurrentStamina = MaxStamina;
    StaminaRegenPerSecond = 30.0f;
    StaminaConsumptionPerSecond = 15.0f;
    RecoveryDelayAfterExhaustion = 2.0f;
	isExhausted = false;
}

void UStaminaComponent::BeginPlay()
{
    Super::BeginPlay();

    SetUpTagManager();

    StartRegenStamina();

	OnStaminaDepletedByTimer.AddDynamic(this, &UStaminaComponent::StartExhaustion);
	OnStaminaDepletedByEvent.AddDynamic(this, &UStaminaComponent::StartExhaustion);
}

void UStaminaComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UStaminaComponent::StartRegenStamina()
{
    if (GetWorld()->GetTimerManager().IsTimerActive(RegenTimerHandle)) {
        return;
    }

    GetWorld()->GetTimerManager().ClearTimer(ConsumeTimerHandle);

    GetWorld()->GetTimerManager().SetTimer(RegenTimerHandle, this, &UStaminaComponent::RegenerateStamina, RegenInterval, true);
}

void UStaminaComponent::StartConsumeStamina()
{
    if (GetWorld()->GetTimerManager().IsTimerActive(ConsumeTimerHandle)) {
        return;
    }

    GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);

    GetWorld()->GetTimerManager().SetTimer(ConsumeTimerHandle, this, &UStaminaComponent::ConsumeStamina, RegenInterval, true);
}
float UStaminaComponent::AddStamina(float Value)
{
    if (!OwnerTagManager || Value < 0 && !OwnerTagManager->GetTags().HasTagExact(FGameplayTag::RequestGameplayTag(FName("StatusEffect.InCombat")))) {
        return CurrentStamina;
    }
	CurrentStamina = FMath::Clamp(CurrentStamina + Value, 0.0f, MaxStamina);
    OnStaminaUpdate.ExecuteIfBound(CurrentStamina / MaxStamina);

	float ResultedStamina = CurrentStamina; // saving just to make sure that it doens't change after call of StartRegenStamina() since need accurate return

    if (CurrentStamina == 0)
    {
        OnStaminaDepletedByEvent.Broadcast();
    }

	if (CurrentStamina == MaxStamina)
	{
		OnReachedMaxStaminaByEvent.Broadcast();
        return ResultedStamina;
    }
    
    StartRegenStamina();
    return ResultedStamina;

}

void UStaminaComponent::ModifyMaxStamina(float Value)
{
	MaxStamina = FMath::Max(MaxStamina + Value, 1.0f);
	OnStaminaUpdate.ExecuteIfBound(CurrentStamina / MaxStamina);
}

void UStaminaComponent::AddToAttribute(float ValueToAdd)
{
    AddStamina(ValueToAdd);
}

void UStaminaComponent::RegenerateStamina()
{
	if (isExhausted || !OwnerTagManager || OwnerTagManager->GetTags().HasAny(RegenBlockingTags)) {
		return;
	}

    if (OwnerTagManager->GetTags().HasAny(StaminaRegenReduceTags)) {
        CurrentStamina = FMath::Min(CurrentStamina + (StaminaRegenPerSecond * RegenInterval) / 10, MaxStamina);
    }
    else {
        CurrentStamina = FMath::Min(CurrentStamina + (StaminaRegenPerSecond * RegenInterval), MaxStamina);
    }

    if (CurrentStamina == MaxStamina) {
        GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);

		OnReachedMaxStaminaByTimer.Broadcast();
    }

	OnStaminaUpdate.ExecuteIfBound(CurrentStamina / MaxStamina);
}

void UStaminaComponent::ConsumeStamina()
{
	if (!OwnerTagManager || !OwnerTagManager->GetTags().HasTagExact(FGameplayTag::RequestGameplayTag(FName("StatusEffect.InCombat")))) {
		return;
    }

    CurrentStamina = FMath::Max(CurrentStamina - (StaminaConsumptionPerSecond * RegenInterval), 0.0f);

    if (CurrentStamina == 0) {
        GetWorld()->GetTimerManager().ClearTimer(ConsumeTimerHandle);

		OnStaminaDepletedByTimer.Broadcast();
    }

    OnStaminaUpdate.ExecuteIfBound(CurrentStamina / MaxStamina);
}

void UStaminaComponent::StartExhaustion()
{
	if (isExhausted) {
		return;
	}
	isExhausted = true;
	GetWorld()->GetTimerManager().SetTimer(ExhaustionTimerHandle, this, &UStaminaComponent::StopExhaustion, RecoveryDelayAfterExhaustion, false);
}

void UStaminaComponent::StopExhaustion()
{
	isExhausted = false;
	GetWorld()->GetTimerManager().ClearTimer(ExhaustionTimerHandle);
}

void UStaminaComponent::SetUpTagManager()
{
	if (ITagManagerOwner* TagManagerOwner = Cast<ITagManagerOwner>(GetOwner()))
	{
		OwnerTagManager = TagManagerOwner->GetTagManager_Implementation();
	}

    if (!OwnerTagManager) {
		FTimerHandle SetUpTagManagerHandle;
		GetWorld()->GetTimerManager().SetTimer(SetUpTagManagerHandle, this, &UStaminaComponent::SetUpTagManager, 0.1f, false);
    }

}

void UStaminaComponent::OnOwnerDeath()
{
    StopExhaustion();
    GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ConsumeTimerHandle);

}
