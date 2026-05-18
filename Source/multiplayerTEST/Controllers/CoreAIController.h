// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "multiplayerTEST/GameplayInterfaces/CombatInterface.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/FormationDecision.h"
#include "CoreAIController.generated.h"

class AEnemyCore;
class UAIPerceptionComponent;
class UEnemyTargetingComponent;
class UGameplayTagManagerComp;

UCLASS()
class MULTIPLAYERTEST_API ACoreAIController : public AAIController, public ICombatInterface
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* AIPerception;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TargetingSystem", meta = (AllowPrivateAccess = "true"))
	UEnemyTargetingComponent* TargetingComponent;

	UPROPERTY()
	UGameplayTagManagerComp* OwnerTagComp; // cache for fast access

public:
	
	ACoreAIController();

	UFUNCTION(BlueprintCallable)
	bool EnableAI();

	UFUNCTION(BlueprintCallable)
	void SetUnderCommand(bool bUnder);

	UFUNCTION(BlueprintPure, Category = "EnemyTarget")
	bool GetIsBoss()const;

	virtual bool bCanStartFight(AActor* Enemy)const override;
	
	// probably should be private but thinking is hard
	UFUNCTION(BlueprintCallable)
	bool CheckSensedActor(AActor* sensedActor);

	void SetUpPawnData(APawn* NewOwningPawn);

	void SetFormationLocation(const FVector& NewLocation);
	void SetFormationRole(EFormationMemberRole NewRole);
	void SetShouldAttackInFormation(bool bShouldAttack);

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBehaviorTree* AIBehaviourTree;

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	bool InitiolizeAI();  // for blackboard initialization

	virtual bool StartFight_Implementation(AActor* EncounterInitiator)override;

	UFUNCTION()
	void OnFightStart();

	UFUNCTION()
	void OnFightEnd();

	void OnPossess(APawn* InPawn) override;


	UPROPERTY(BlueprintReadOnly)
	AEnemyCore* OwningPawn;
	
};
