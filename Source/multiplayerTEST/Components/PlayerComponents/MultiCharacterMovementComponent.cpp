// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/AnimInstances/PlayerAnimInstance.h"
#include "multiplayerTEST/GameplayInterfaces/PlayerInformationInterface.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/GhostAnimInstanceSyncInterface.h"

void UMultiCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<ACharacter>(GetOwner());
}

void UMultiCharacterMovementComponent::PerformMovement(float DeltaTime)
{
	Super::PerformMovement(DeltaTime);
	
	if (!OwnerChar || !RotateCharacter)
		return;

	if (OwnerChar->GetLocalRole() != ROLE_Authority && !OwnerChar->IsLocallyControlled()) // owner or server only
	{
		return;
	}

	if (LockOnMesh) {
		SetActorRotation(LockOnMesh->GetSocketLocation(LockOnSocket) - OwnerChar->GetActorLocation(), DeltaTime);
		return;
	}

	FVector InputVector = GetCurrentAcceleration().GetSafeNormal();
	
	SetActorRotation(InputVector, DeltaTime);
}

FVector UMultiCharacterMovementComponent::ConstrainAnimRootMotionVelocity(
	const FVector& RootMotionVelocity,
	const FVector& CurrentVelocity
) const
{
	FVector Result = RootMotionVelocity;

	if (IsFalling())
	{
		// If animation is NOT pushing upward, allow gravity to affect Z
		if (RootMotionVelocity.Z <= 0.f)
		{
			Result.Z = CurrentVelocity.Z;
		}
		// else: animation fully controls upward motion
	}

	return Result;
}

void UMultiCharacterMovementComponent::SetActorRotation(FVector InputVector, float DeltaTime)
{
	if (InputVector.IsNearlyZero()) {
		return;
	}
	
	FRotator DesiredRotation = InputVector.Rotation();

	FRotator CurrentRotation = OwnerChar->GetActorRotation();

	float RotationMultiplier = 1.0f; // Default multiplier
	if (currentRotateTotalTime < 0.4f)
	{
		RotationMultiplier = 0.4f / currentRotateTotalTime; // Scale up if the time is less than 0.4s
	}
	RotationMultiplier += 0.4;
	FRotator SmoothRotation = FMath::RInterpConstantTo(CurrentRotation, FRotator(0.0f, DesiredRotation.Yaw, 0.0f), DeltaTime, 360.0f * RotationMultiplier); // Use constant interpolation
	OwnerChar->SetActorRotation(SmoothRotation);	
}

void UMultiCharacterMovementComponent::SetRotateCharacter(bool bRotateCharacter, float TotalRotateTime)
{
	if (GetOwner()->GetLocalRole() != ROLE_Authority) {
		SetRotateCharacterServer(bRotateCharacter, TotalRotateTime);
	}
	

	RotateCharacter = bRotateCharacter;
	currentRotateTotalTime = TotalRotateTime;
}

void UMultiCharacterMovementComponent::SetRotateCharacterServer_Implementation(bool bRotateCharacter, float TotalRotateTime)
{
	SetRotateCharacter(bRotateCharacter, TotalRotateTime);
}

void UMultiCharacterMovementComponent::OnMovementModeChanged(
	EMovementMode PreviousMovementMode,
	uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (!OwnerChar || !OwnerTagManagerComp) return;

	if (IsFalling())
	{
		if (UPlayerAnimInstance* OwnerAnimInstance = Cast<UPlayerAnimInstance>(OwnerChar->GetMesh()->GetAnimInstance())) {
			OwnerAnimInstance->bShouldRollOnJumpToBonfire = FMath::FRand() <= OwnerAnimInstance->ChanceToRollOnJumpToBonfire;
		}

		OwnerTagManagerComp->AddTag(IsFallingTag);
		return;
	}

	OwnerTagManagerComp->RemoveTag(IsFallingTag);

	if (OwnerChar->Implements<UPlayerInformationInterface>())
	{
		IPlayerInformationInterface::Execute_TryLastInputAction(OwnerChar);
		return;
	}

	if(IGhostAnimInstanceSyncInterface* CharInterFaceGhost = Cast<IGhostAnimInstanceSyncInterface>(OwnerChar))
	{
		ACharacter* ActualOwner = CharInterFaceGhost->GetPossessedGhostCharacter();

		if (ActualOwner && ActualOwner->Implements<UPlayerInformationInterface>())
		{
			IPlayerInformationInterface::Execute_TryLastInputAction(ActualOwner);
		}
	}
}
