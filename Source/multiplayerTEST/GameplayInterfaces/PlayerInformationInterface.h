// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInformationInterface.generated.h"

class UWearableItem;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInformationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MULTIPLAYERTEST_API IPlayerInformationInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "InteractInterface")
	UWearableItem* GetHelmet();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "InteractInterface")
	UWearableItem* GetChest();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "InteractInterface")
	UWearableItem* GetLegs();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "InteractInterface")
	UWearableItem* GetWeaponRH1();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "InteractInterface")
	UWearableItem* GetWeaponLH1();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void TryLastInputAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetbEndedFirstRollAttackToTrue();
};
