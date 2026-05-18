// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TagManagerOwner.generated.h"

class UGameplayTagManagerComp;

UINTERFACE(BlueprintType)
class UTagManagerOwner : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MULTIPLAYERTEST_API ITagManagerOwner
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "TagManager")
	UGameplayTagManagerComp* GetTagManager() const;
};
