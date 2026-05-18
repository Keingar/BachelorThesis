// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldSpaceHealthBar.h"
#include "PlayerWorldSpaceHealthBar.generated.h"

class UTextBlock;
class UHealthComponent;

UCLASS()
class MULTIPLAYERTEST_API UPlayerWorldSpaceHealthBar : public UWorldSpaceHealthBar
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* NicknameText;

public:
	virtual void NativeConstruct() override;

	void SetUpHealthDelegate(UHealthComponent* OwningHealthComponent) override;

	UFUNCTION(BlueprintCallable)
	void SetUpNickname(FText PlayerNickname, AActor* LocalPlayerr);
protected:
	void CheckObstacles();

	TWeakObjectPtr<AActor> WidgetOwner;
	AActor* LocalPlayer;

	virtual void SetDesiredVisibility() override;
	UFUNCTION()
	virtual void OnSomeoneDeath(AActor* DeadActor);

	FTimerHandle CheckOnstaclesTimer;

	virtual void SetUpOnSomeoneDeath(UHealthComponent* OwningHealthComponent);
};
