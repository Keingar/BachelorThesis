// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShipPassangerGhostCore.h"
#include "ShipEnemyPassangerGhost.generated.h"

class UEnemyTargetingComponent;

UCLASS()
class MULTIPLAYERTEST_API AShipEnemyPassangerGhost : public AShipPassangerGhostCore
{
	GENERATED_BODY()
	
protected:
	virtual void Tick(float DeltaTime)override;

public:
	AShipEnemyPassangerGhost(const FObjectInitializer& ObjectInitializer);

	UPROPERTY()
	UEnemyTargetingComponent* OwnerTargetComp;
	float RotationSpeed;

};
