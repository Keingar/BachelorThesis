// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GhostAnimInstanceSyncInterface.generated.h"

UINTERFACE(MinimalAPI)
class UGhostAnimInstanceSyncInterface : public UInterface
{
	GENERATED_BODY()
};

// interface created to get mesh and correct anim instance inside visual ship
// to assing correct anim instance to ghost since its different for each skeletal mesh

class MULTIPLAYERTEST_API IGhostAnimInstanceSyncInterface
{
	GENERATED_BODY()

public:

	virtual TSubclassOf<UAnimInstance> GetGhostAnimInstanceClass() const = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	ACharacter* GetPossessedGhostCharacter() const;
};
