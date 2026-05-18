// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MultiplayerTeestHUD.generated.h"

/**
 * 
 */
class UFullUIHUD;
class AEnemyCore;

UCLASS()
class MULTIPLAYERTEST_API AMultiplayerTeestHUD : public AHUD
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void RemoveAllWidgetsEvent();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ResetUIAfterBonfire();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DrawWidgets();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DrawBossHealthBar(AEnemyCore* CurrentBoss);

	UFUNCTION(BlueprintImplementableEvent)
	void DrawBonfireWidget(FName BonepileID);

	UFUNCTION(BlueprintImplementableEvent)
	void DrawEscMenu();

	UFUNCTION(BlueprintImplementableEvent)
	void CloseBonfireWidget();


	UFUNCTION()
	void RemoveBossHealthBar(AEnemyCore* CurrentBoss, bool isBossDefeated);
	
	UFUNCTION(BlueprintCallable)
	UFullUIHUD* GetFullUIHUD() const { return FullUIHUD; }

	UFUNCTION(BlueprintImplementableEvent)
	void DrawCanRestAtBonfireUI();

	UFUNCTION(BlueprintImplementableEvent)
	void CloseCanRestAtBonfireUI();

protected:

	UPROPERTY(BlueprintReadWrite)
	UFullUIHUD* FullUIHUD;

	virtual void BeginPlay() override;
};
	