// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "MultiCheatManager.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UMultiCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UFUNCTION(exec)
	void GiveAllItems();

	UFUNCTION(exec)
	void GetItemByID(int ItemID);

	UFUNCTION(exec)
	void OpenAllBonepiles();

	UFUNCTION(exec)
	void ResetBossess();
};
