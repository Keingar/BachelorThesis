// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ControllerInfo.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UControllerInfo : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MULTIPLAYERTEST_API IControllerInfo
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ControllerInfo")
	bool const GetIsOpenInventory();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ControllerInfo")
	void SetIsOpenInventory(bool newVal);

};
