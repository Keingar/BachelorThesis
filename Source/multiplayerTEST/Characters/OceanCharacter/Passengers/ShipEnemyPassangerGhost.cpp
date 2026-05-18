// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipEnemyPassangerGhost.h"
#include "multiplayerTEST/Components/EnemyComponents/EnemyTargetingComponent.h"

AShipEnemyPassangerGhost::AShipEnemyPassangerGhost(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AShipEnemyPassangerGhost::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!OwnerTargetComp || RotationSpeed == 0) return;

	AActor* CurrentTarget = OwnerTargetComp->GetCurrentGhostTarget();

	if (!CurrentTarget) return;

	FRotator DesiredRotation = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D().Rotation();

	SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), DesiredRotation, DeltaTime, RotationSpeed));
}