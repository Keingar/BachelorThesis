// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "NameTagComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIPLAYERTEST_API UNameTagComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNameTagComponent();

protected:
	FText NameTagText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NameTag")
	TSubclassOf<UUserWidget> NameTagWidgetClass;

public:

	FTimerHandle CheckNameTagDesiredVisibilityTimer;

	void CheckNameTagDesiredVisibility();

	UFUNCTION(BlueprintCallable)
	void SetUpNameTags();
};
