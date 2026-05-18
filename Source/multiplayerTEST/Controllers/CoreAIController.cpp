// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "multiplayerTEST/Components/EnemyComponents/EnemyTargetingComponent.h"
#include "multiplayerTEST/Characters/Enemies/EnemyCore.h"
#include "multiplayerTEST/Components/EnemyComponents/GroupCommandComponent.h"
#include "multiplayerTEST/GameModeRelated/multiplayerTESTGameMode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/TagManagerOwner.h"
#include "multiplayerTEST/Characters/OceanCharacter/Passengers/ShipEnemyPassangerGhost.h"
#include "BehaviorTree/BlackboardComponent.h"

ACoreAIController::ACoreAIController()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>("AIPerception");

	TargetingComponent = CreateDefaultSubobject<UEnemyTargetingComponent>("TargetingComp");
}

void ACoreAIController::BeginPlay()
{
	Super::BeginPlay();

	TargetingComponent->OnFightStarted.AddDynamic(this, &ACoreAIController::OnFightStart);

	TargetingComponent->OnFigthStopped.AddDynamic(this, &ACoreAIController::OnFightEnd);
}

bool ACoreAIController::CheckSensedActor(AActor* sensedActor)
{
	if (!sensedActor || sensedActor == GetOwner()) {
		return false; // invalid sensed actor
	}

	ICombatInterface* combatInterface = Cast<ICombatInterface>(sensedActor);
	AActor* realActor = sensedActor;

	// if sensed not valid then check if its ghost and work with that
	if (!combatInterface) {
		AShipPassangerGhostCore* ghostInterface = Cast<AShipPassangerGhostCore>(sensedActor);

		if (!ghostInterface) return false;

		realActor = ghostInterface->RealActor;
		
		if (!realActor || realActor == GetOwner()) {
			return false; // invalid sensed actor
		}

		combatInterface = Cast<ICombatInterface>(realActor);

		if (!combatInterface) return false;
	}

	if (!OwnerTagComp) {
		return false; 
	}

	if (!combatInterface->bCanStartFight(this)) {
		return false; // fight denied by other actor
	}

	UGameplayTagManagerComp* SensedTagComp = nullptr;

	if (ITagManagerOwner * sensedOwnerOfTagManager = Cast<ITagManagerOwner>(realActor)) {
		SensedTagComp = sensedOwnerOfTagManager->GetTagManager_Implementation();
	}

	if (!SensedTagComp) { return false; }

	if (!OwnerTagComp->GetOwnerEnemiesFactionTags().HasTag(SensedTagComp->GetOwnerFactionTag())) {
		return false; // not an enemy
	}

	if (!bCanStartFight(realActor)) {
		return false; // fight denied
	}

	if (!EnableAI()) {
		return false;
	}

	if (!TargetingComponent->StartFight(realActor)) { // usually will fail only if in fight with sensedActor already
		return false;
	}

	if (SensedTagComp->GetOwnerFactionTag() == FGameplayTag::RequestGameplayTag(FName("FactionType.Player"))) {
		combatInterface->StartFight_Implementation(OwningPawn);
	}

	return true; // true if AI was enabled
}

void ACoreAIController::SetUpPawnData(APawn* NewOwningPawn)
{
	if (!NewOwningPawn) {
		UE_LOG(LogTemp, Error, TEXT("CoreAIController has no pawn at begin play! %s"), *GetName());
		return;
	}
	OwningPawn = Cast<AEnemyCore>(NewOwningPawn);

	OwnerTagComp = OwningPawn->GetComponentByClass<UGameplayTagManagerComp>();

	if (!OwnerTagComp) {
		UE_LOG(LogTemp, Error, TEXT("Couldn't get owner tags component for %s"), *GetName());
	}

	TargetingComponent->SetUpPawnDelegates(OwningPawn);
}

bool ACoreAIController::EnableAI()
{
	if (!OwningPawn) return false;

	if (!InitiolizeAI()) { 
		UE_LOG(LogTemp, Error, TEXT("Can't enable AI for %s"), *GetName());

		return false; // something went wrong in blueprint
	}

	OwningPawn->bNeedReset = true;

	RunBehaviorTree(AIBehaviourTree);

	if (Blackboard)
	{
		Blackboard->SetValueAsObject(TEXT("RealSelfActor"), OwningPawn);
		Blackboard->SetValueAsBool(TEXT("bShouldAttackInFormation"), false);
	}

	return true;
}

bool ACoreAIController::GetIsBoss() const
{
	if (OwningPawn) {
		return OwningPawn->GetIsBoss();
	}

	return false;
}

bool ACoreAIController::StartFight_Implementation(AActor* EncounterInitiator)
{
	return CheckSensedActor(EncounterInitiator);
}

bool ACoreAIController::bCanStartFight(AActor* Enemy)const
{
	if (!OwningPawn) {
		UE_LOG(LogTemp, Warning, TEXT("Invalid OwnerRef in %s"), *this->GetName());

		return false;
	}

	return OwningPawn->bCanStartFight();
}

void ACoreAIController::OnFightStart()
{
    // Only mark under command if this pawn actually belongs to a group that is in combat.
	if (OwningPawn && OwningPawn->OwningGroupCommandComponent)
	{
		SetUnderCommand(true);
	}
}

void ACoreAIController::OnFightEnd()
{
}

void ACoreAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AEnemyCore* NewPawn = Cast<AEnemyCore>(InPawn);
	if (!NewPawn) return;

	OwningPawn = NewPawn;

	SetUpPawnData(OwningPawn);

	OwningPawn->OwnerTargetComp = TargetingComponent;

	OwningPawn->OverlapActorsAtSpawn();
}

void ACoreAIController::SetFormationLocation(const FVector& NewLocation)
{
	if (!Blackboard) return;

	Blackboard->SetValueAsVector(TEXT("FormationLocation"), NewLocation);
}

void ACoreAIController::SetFormationRole(EFormationMemberRole NewRole)
{
	if (!Blackboard) return;

	Blackboard->SetValueAsEnum(TEXT("FormationRole"), static_cast<uint8>(NewRole));
}

void ACoreAIController::SetShouldAttackInFormation(bool bShouldAttack)
{
	if (!Blackboard) return;

	Blackboard->SetValueAsBool(TEXT("bShouldAttackInFormation"), bShouldAttack);
}

void ACoreAIController::SetUnderCommand(bool bUnder)
{
	if (!Blackboard) return;

	Blackboard->SetValueAsBool(TEXT("bUnderCommand"), bUnder);
}
