// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBase.generated.h"

class UProgressBar;
class UHealthComponent;

UCLASS()
class MULTIPLAYERTEST_API UHealthBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UProgressBar* PreviousHealthBar;

	void UpdatePreviousHealtBar();

	FTimerHandle PreviousHealthbarMachingTimer;

	UFUNCTION()
	virtual void SetHealth(float CurrentHealth, float MaxHealth);

	virtual void CalcHealthPercentage(float CurrentHealth, float MaxHealth);

	UHealthComponent* OwnerHealthComponent;

	void SetUpHealthStuff();

	FTimerHandle HandleToSetuPHealthStuff;
public:
	UFUNCTION(BlueprintCallable)
	virtual void SetUpHealthDelegate(UHealthComponent* OwningHealthComponent);

};
