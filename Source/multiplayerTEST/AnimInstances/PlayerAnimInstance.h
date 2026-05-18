// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/GhostAnimSyncInterface.h"
#include "PlayerAnimInstance.generated.h"

class UCharacterMovementComponent;

UCLASS()
class MULTIPLAYERTEST_API UPlayerAnimInstance : public UAnimInstance, public IGhostAnimSyncInterface
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	bool bWantToRestAtBonfire = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsChangingRestState = false;
	UPROPERTY(BlueprintReadOnly)
	bool bInBonfireInteractionRadius = false;

	UPROPERTY(BlueprintReadOnly)
	bool bShouldRollOnJumpToBonfire;
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="0", ClampMax="1"))
	float ChanceToRollOnJumpToBonfire = 0.3f;

	virtual void SetGhostMovementComp(class UCharacterMovementComponent* NewCharMoveComp) override {
		GhostCharMovementComp = NewCharMoveComp;
	}

	UPROPERTY(BlueprintReadWrite)
	bool WantToBlock;
	UPROPERTY(BlueprintReadOnly)
	bool TwoHandWeapon;

protected:
	UPROPERTY(BlueprintReadOnly)
	UCharacterMovementComponent* GhostCharMovementComp;
};
