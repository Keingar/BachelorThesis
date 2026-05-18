// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPassangerGhostCore.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "multiplayerTEST/Characters/OceanCharacter/Ships/Ship_Ghost.h"
#include "multiplayerTEST/Characters/OceanCharacter/Ships/Ship_Visual.h"

AShipPassangerGhostCore::AShipPassangerGhostCore(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	SetTickGroup(ETickingGroup::TG_EndPhysics); // so its after ship bouyancy to use later floor info

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
}

void AShipPassangerGhostCore::BeginPlay()
{
	Super::BeginPlay();

	if (GetMesh())
	{
		GetMesh()->SetVisibility(false, true);
	}
}

void AShipPassangerGhostCore::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || !VisualShip || !GhostShip || !RealActor)
	{
		return;
	}

	const FTransform GhostLocal =
		GetActorTransform().GetRelativeTransform(GhostShip->GetActorTransform());

	const FTransform FinalWorld =
		GhostLocal * VisualShip->GetActorTransform();

	RealActor->SetActorTransform(
		FinalWorld,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);
}

void AShipPassangerGhostCore::CopySettingsFromActor(ACharacter* RealCharacter)
{
	if (!RealCharacter)
	{
		return;
	}

	UCharacterMovementComponent* SourceMove = RealCharacter->GetCharacterMovement();
	UCharacterMovementComponent* TargetMove = GetCharacterMovement();

	if (!SourceMove || !TargetMove)
	{
		return;
	}

	// Walking
	TargetMove->MaxWalkSpeed = SourceMove->MaxWalkSpeed;
	TargetMove->MaxWalkSpeedCrouched = SourceMove->MaxWalkSpeedCrouched;
	TargetMove->MaxAcceleration = SourceMove->MaxAcceleration;
	TargetMove->BrakingDecelerationWalking = SourceMove->BrakingDecelerationWalking;
	TargetMove->GroundFriction = SourceMove->GroundFriction;
	TargetMove->BrakingFrictionFactor = SourceMove->BrakingFrictionFactor;
	TargetMove->BrakingFriction = SourceMove->BrakingFriction;

	// Jump / Air
	TargetMove->JumpZVelocity = SourceMove->JumpZVelocity;
	TargetMove->AirControl = SourceMove->AirControl;
	TargetMove->AirControlBoostMultiplier = SourceMove->AirControlBoostMultiplier;
	TargetMove->AirControlBoostVelocityThreshold = SourceMove->AirControlBoostVelocityThreshold;
	TargetMove->FallingLateralFriction = SourceMove->FallingLateralFriction;
	TargetMove->GravityScale = SourceMove->GravityScale;

	// Rotation / Movement behavior
	TargetMove->bOrientRotationToMovement = SourceMove->bOrientRotationToMovement;
	TargetMove->RotationRate = SourceMove->RotationRate;

	// Step / floor interaction
	TargetMove->MaxStepHeight = SourceMove->MaxStepHeight;
	TargetMove->PerchRadiusThreshold = SourceMove->PerchRadiusThreshold;
	TargetMove->PerchAdditionalHeight = SourceMove->PerchAdditionalHeight;
	TargetMove->SetWalkableFloorAngle(SourceMove->GetWalkableFloorAngle());
	TargetMove->SetWalkableFloorZ(SourceMove->GetWalkableFloorZ());

	TargetMove->Velocity = SourceMove->Velocity;
	TargetMove->SetMovementMode(SourceMove->MovementMode);

}