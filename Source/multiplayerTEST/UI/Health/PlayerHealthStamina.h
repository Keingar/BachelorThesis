// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBase.h"
#include "PlayerHealthStamina.generated.h"

class UProgressBar;

UCLASS()
class MULTIPLAYERTEST_API UPlayerHealthStamina : public UHealthBase
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UProgressBar* StaminaBar;

	void UpdateStaminaBar(float newPercent);
	UFUNCTION(BlueprintCallable)
	void SetUpStaminaDelegate();

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void ChangeSkullIcon(int NewHealthPercent);

	void CalcHealthPercentage(float CurrentHealth, float MaxHealth) override;

	void SetUpHealthDelegateTimer();
};
