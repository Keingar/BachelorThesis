// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerWorldSpaceHealthBar.h"
#include "OtherPlayerHealthHudUI.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UOtherPlayerHealthHudUI : public UPlayerWorldSpaceHealthBar
{
	GENERATED_BODY()


public:
	UPROPERTY(BlueprintReadWrite)
	APlayerState* PlayerStateOwner;
	virtual void NativeConstruct() override;

protected:
	void OnSomeoneDeath(AActor* DeadActor)override;

	virtual void SetDesiredVisibility() override;
	virtual void SetUpOnSomeoneDeath(UHealthComponent* OwningHealthComponent)override;

};
