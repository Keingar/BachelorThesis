// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/GhostAnimSyncInterface.h"
#include "EnemyAnimInstanceCore.generated.h"


UCLASS()
class MULTIPLAYERTEST_API UEnemyAnimInstanceCore : public UAnimInstance, public IGhostAnimSyncInterface
{
	GENERATED_BODY()
	
public:
	virtual void SetGhostMovementComp(class UCharacterMovementComponent* NewCharMoveComp) override {
		GhostCharMovementComp = NewCharMoveComp;
	}

protected:
	UPROPERTY(BlueprintReadOnly)
	UCharacterMovementComponent* GhostCharMovementComp;
};
