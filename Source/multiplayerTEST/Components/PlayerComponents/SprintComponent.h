// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SprintComponent.generated.h"

class UStaminaComponent;
class UGameplayTagManagerComp;
class UCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSprintEvent);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIPLAYERTEST_API USprintComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USprintComponent();

protected:
	virtual void BeginPlay() override;
	
public:

	UPROPERTY(BlueprintAssignable)
	FSprintEvent OnSprintStarted;

	UPROPERTY(BlueprintAssignable)
	FSprintEvent OnSprintStopped;

	UPROPERTY()
	UCharacterMovementComponent* GhostMovementComp; // valid only on ship

protected:

	UPROPERTY()
	UStaminaComponent* StaminaComponent;
	UPROPERTY()
	UGameplayTagManagerComp* TagManager;
	UPROPERTY()
	UCharacterMovementComponent* OwnerMovementComponent;
	UPROPERTY(EditDefaultsOnly)
	float SprintSpeed;
	UPROPERTY(EditDefaultsOnly)
	float WalkSpeed;

public:

	FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }
	FORCEINLINE float GetSprintSpeed() const { return SprintSpeed; }

	bool TryStartSprint();

	UFUNCTION()
	void TryStopSprint();

	bool TrySprintFromMoving(); // only to start run when input started before moving

	bool CanSprint() const;

	void SetUpData(UStaminaComponent* NewStaminaComp, UGameplayTagManagerComp* NewTagManager, UCharacterMovementComponent* NewMoveComp);

protected:
	bool bWantToSprint;

	bool SprintFunction();

	void StopSprintFunction();

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag SprintTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag MoveTag;
};
