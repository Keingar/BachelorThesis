// Fill out your copyright notice in the Description page of Project Settings.


#include "SprintComponent.h"
#include "multiplayerTEST/Components/StaminaComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"

USprintComponent::USprintComponent()
{
	bWantToSprint = false;

	WalkSpeed = 400.f;
	SprintSpeed = 600.f;
}

void USprintComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USprintComponent::SetUpData(UStaminaComponent* NewStaminaComp, UGameplayTagManagerComp* NewTagManager, UCharacterMovementComponent* NewMoveComp)
{
	StaminaComponent = NewStaminaComp;

	TagManager = NewTagManager;

	OwnerMovementComponent = NewMoveComp;

	OwnerMovementComponent->MaxWalkSpeed = WalkSpeed;

	StaminaComponent->OnStaminaDepletedByTimer.AddDynamic(this, &USprintComponent::TryStopSprint);
}

bool USprintComponent::TrySprintFromMoving()
{
	if (!TagManager) return false;

	TagManager->AddTag(MoveTag);

	if (bWantToSprint) {

		return TryStartSprint();
	}

	return false;
}

bool USprintComponent::CanSprint() const
{
	if (!TagManager) return false;

	if (StaminaComponent->GetCurrentStamina() == 0 || !bWantToSprint || !TagManager->GetTags().HasTagExact(MoveTag))
	{
		return false;
	}

	return true;
}

bool USprintComponent::TryStartSprint()
{
	bWantToSprint = true;

	if (SprintFunction()) {
		return true;
	}	
	return false;
}

void USprintComponent::TryStopSprint()
{
	bWantToSprint = false;

	if (!TagManager || !TagManager->GetTags().HasTagExact(SprintTag)) {
		return;
	}

	StopSprintFunction();
}

bool USprintComponent::SprintFunction() {
	if (!CanSprint() || !OwnerMovementComponent) {
		return false;
	}

	OwnerMovementComponent->MaxWalkSpeed = SprintSpeed;

	if (GhostMovementComp) {
		GhostMovementComp->MaxWalkSpeed = SprintSpeed;
	}

	StaminaComponent->StartConsumeStamina();
	TagManager->RemoveTag(MoveTag);
	TagManager->AddTag(SprintTag);

	return true;
}

void USprintComponent::StopSprintFunction()
{
	if (!OwnerMovementComponent || !TagManager) return;

	OwnerMovementComponent->MaxWalkSpeed = WalkSpeed;

	if (GhostMovementComp) {
		GhostMovementComp->MaxWalkSpeed = WalkSpeed;
	}

	TagManager->RemoveTag(SprintTag);
	TagManager->AddTag(MoveTag);
	

	StaminaComponent->StartRegenStamina();
}
