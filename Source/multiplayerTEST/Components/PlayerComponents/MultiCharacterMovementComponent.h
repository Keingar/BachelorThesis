// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"
#include "MultiCharacterMovementComponent.generated.h"

class UGameplayTagManagerComp;

UCLASS()
class MULTIPLAYERTEST_API UMultiCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay()override;

	virtual void PerformMovement(float DeltaTime) override;

	virtual FVector ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const override;

	void SetActorRotation(FVector InputVector, float DeltaTime);

	virtual void OnMovementModeChanged(
		EMovementMode PreviousMovementMode,
		uint8 PreviousCustomMode
	) override;
public:
	bool RotateCharacter;
	float currentRotateTotalTime;

	FName LockOnSocket;
	USkeletalMeshComponent* LockOnMesh;
	UPROPERTY()
	UGameplayTagManagerComp* OwnerTagManagerComp;

	UFUNCTION(BlueprintCallable)
	void SetRotateCharacter(bool bRotateCharacter, float TotalRotateTime); // used inside CanTurn notify state

protected:
	ACharacter* OwnerChar;

	UFUNCTION(Server, Reliable)
	void SetRotateCharacterServer(bool bRotateCharacter, float TotatlRotateTime);

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag IsFallingTag;
};
