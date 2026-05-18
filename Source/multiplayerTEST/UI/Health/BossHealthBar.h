// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBase.h"
#include "BossHealthBar.generated.h"

class UTextBlock;
class UHealthComponent;

UCLASS()
class MULTIPLAYERTEST_API UBossHealthBar : public UHealthBase
{
	GENERATED_BODY()
	


public:

	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* BossName;

	UFUNCTION(BlueprintCallable)
	void SetUpHealthBar(UHealthComponent* OwnerHealthComp, FText CurrentBossName);

};
