// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBase.h"
#include "WorldSpaceHealthBar.generated.h"

class UTextBlock;
class UHealthComponent;

UCLASS()
class MULTIPLAYERTEST_API UWorldSpaceHealthBar : public UHealthBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	void SetHealth(float CurrentHealth, float MaxHealth)override;

	void SetVisibilityAndText(float CurrentHealth, float MaxHealth);

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* HealthText;

	void SetChangeHealthRecentlyFalse();

	bool bisLockedOn;
	bool changedHealthRecently;

	float HealthDifference;

	float PreviuosHealth;

	virtual void SetDesiredVisibility();

	void SetisLockedOnInternal(bool bIsLockedOn);

	void SetHealthTextNotVisible();

	FTimerHandle ChangeHealthVisibilityTimer;

	FTimerHandle ChangeLockonVisibilityTimer;

	FTimerHandle HealthTextVisibilityTimer;

	FTimerHandle PreviousHealthbarMachingTimer;

public:
	void SetLockedOn(bool bIsLockedOn);

	virtual void SetUpHealthDelegate(UHealthComponent* OwningHealthComponent)override;

};
