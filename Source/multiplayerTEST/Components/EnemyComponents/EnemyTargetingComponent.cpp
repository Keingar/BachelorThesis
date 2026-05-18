// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyTargetingComponent.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "multiplayerTEST/Controllers/CoreAIController.h"
#include "multiplayerTEST/Components/ImpactComponent.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/GhostAnimInstanceSyncInterface.h"
#include "TimerManager.h"

UEnemyTargetingComponent::UEnemyTargetingComponent()
{
	canSwitchByTime = true;
	FocusedAgroTime = 5;
	isInitiolized = false;
}

void UEnemyTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UEnemyTargetingComponent::ChangeTarget(AActor* NewEnemy, float HitAngleInDegrees) // angle is here just to bind onGotHit
{
	// certainly not the best place to do this logic but but if it works it works 
	if (!ListOfEnemies.Contains(NewEnemy)) {
		if (ACoreAIController* TempOwnerControllerSave = Cast<ACoreAIController>(GetOwner())) {
			TempOwnerControllerSave->CheckSensedActor(NewEnemy);
		}
		return;
	}

	if (!canSwitchByTime || NewEnemy == CurrentEnemy) {
		return;
	}

	CurrentEnemy = NewEnemy;

	if (IGhostAnimInstanceSyncInterface* GhostSync = Cast<IGhostAnimInstanceSyncInterface>(CurrentEnemy)) {
		CurrentGhostEnemy = GhostSync->GetPossessedGhostCharacter_Implementation();
	}
	else {
		CurrentGhostEnemy = nullptr;
	}

	canSwitchByTime = false;

	OnTargetChanged.Broadcast(CurrentEnemy);

	SetUpFocusAgroTimer();
}

void UEnemyTargetingComponent::TargetDispatcher()
{
	OnTargetChanged.Broadcast(CurrentEnemy);
}
	
void UEnemyTargetingComponent::OnAgroTimerEnd()
{
	canSwitchByTime = true;
}

void UEnemyTargetingComponent::SetUpFocusAgroTimer()
{
	GetWorld()->GetTimerManager().SetTimer(AgroTimer, this, &UEnemyTargetingComponent::OnAgroTimerEnd, FocusedAgroTime, false);
}

void UEnemyTargetingComponent::OnTargetDied(AActor* DeadEnemy)
{
	
	ListOfEnemies.Remove(DeadEnemy);

	DeadEnemy->GetComponentByClass<UHealthComponent>()->ActorDied.RemoveDynamic(this, &UEnemyTargetingComponent::OnTargetDied);

	if (ListOfEnemies.IsEmpty()) {

		bIsFighting = false;
		OnFigthStopped.Broadcast();

		return;
	}

	if (CurrentEnemy == DeadEnemy) {
		ChangeTarget(ListOfEnemies[0]);
	}
}

void UEnemyTargetingComponent::OnOwnerDied(AActor* DeadOwner)
{

	if (bIsFighting) {
		ListOfEnemies.Empty();

		bIsFighting = false;

		OnFigthStopped.Broadcast();
	}
}

void UEnemyTargetingComponent::SetUpPawnDelegates(APawn* OwnerPawn)
{
	if (isInitiolized) return;

	if (!OwnerPawn) {
		UE_LOG(LogTemp, Error, TEXT("Owner of EnemyTargetComponent has no pawn. (probably change from pawn to controller) inside %s"), *GetName());
		return;
	}

	isInitiolized = true;

	if (UHealthComponent* OwnerHealhComponent = OwnerPawn->GetComponentByClass<UHealthComponent>()) {
		OwnerHealhComponent->ActorDied.AddDynamic(this, &UEnemyTargetingComponent::OnOwnerDied);
	}

	if (UImpactComponent* OwnerImpactComponent = OwnerPawn->GetComponentByClass<UImpactComponent>()) {
		OwnerImpactComponent->OnGotHit.AddDynamic(this, &UEnemyTargetingComponent::ChangeTarget);
	}
}

bool UEnemyTargetingComponent::StartFight(AActor* NewEnemy)
{
	if (ListOfEnemies.Contains(NewEnemy)) {
		return false; // already fighting this enemy
	}

	if (UHealthComponent* EnemyHealhComponent = NewEnemy->GetComponentByClass<UHealthComponent>()) {
		EnemyHealhComponent->ActorDied.AddDynamic(this, &UEnemyTargetingComponent::OnTargetDied);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Enemy has no health component can't start fight"));

		return false;
	}
	ListOfEnemies.Add(NewEnemy); // we already checked for uniqueness

	OnNewEnemy.Broadcast(NewEnemy);

	if (!bIsFighting) { // broadcast only when started first time

		bIsFighting = true;
		
		ChangeTarget(NewEnemy); 

		OnFightStarted.Broadcast();

	}

	return true;
}
