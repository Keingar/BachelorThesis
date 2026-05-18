// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnvironmentInteractInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEnvironmentInteractInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MULTIPLAYERTEST_API IEnvironmentInteractInterface
{
	GENERATED_BODY()

public:
	virtual void TeleportActor(FVector Destination, FRotator NewRotation, bool DisableMovement = false) = 0;

};
