// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FullUIHUD.generated.h"

/**
 * 
 */
class UPlayerHealthStamina;
class UEscMenu;
class UHealthBase;

UCLASS()
class MULTIPLAYERTEST_API UFullUIHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Ensure the widget names in the UMG blueprint match these property names
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	UPlayerHealthStamina* HealthStamina;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	UEscMenu* EscMenu;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	UHealthBase* BossHealthBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	UUserWidget* BonfireMenuWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	UUserWidget* CanRestUI;
};
