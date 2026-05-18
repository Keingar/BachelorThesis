// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyTargetingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChangedTarget, AActor*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFightStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFightStop);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewEnemy, AActor*, NewEnemyActor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTEST_API UEnemyTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyTargetingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOwnerDied(AActor* DeadOwner);

	bool isInitiolized;

public:
	UFUNCTION(BlueprintPure, Category = "EnemyTarget")
	FORCEINLINE AActor* GetCurrentTarget() const { return CurrentEnemy; };
	
	UFUNCTION(BlueprintPure, Category = "EnemyTarget")
	FORCEINLINE ACharacter* GetCurrentGhostTarget() const { return CurrentGhostEnemy; };

	bool IsFightingActor(AActor* Actor) const { return ListOfEnemies.Contains(Actor); }

	UPROPERTY(BlueprintAssignable)
	FFightStart OnFightStarted;

	UPROPERTY(BlueprintAssignable)
	FFightStop OnFigthStopped;

	UPROPERTY(BlueprintAssignable)
	FNewEnemy OnNewEnemy;

	UPROPERTY(BlueprintAssignable)
	FChangedTarget OnTargetChanged;

protected:
	AActor* CurrentEnemy;
	ACharacter* CurrentGhostEnemy;

	TArray<AActor*> ListOfEnemies;

	UFUNCTION()
	void ChangeTarget(AActor* NewEnemy, float HitAngleInDegrees = 0);

	UPROPERTY(EditAnywhere, Category = "EnemyTarget")
	int FocusedAgroTime;

	void TargetDispatcher();

	bool canSwitchByTime;

	FTimerHandle AgroTimer;

	void OnAgroTimerEnd();

	void SetUpFocusAgroTimer();

	UFUNCTION()
	void OnTargetDied(AActor* DeadEnemy);

	bool bIsFighting; // for public tags can be used

public:
	void SetUpPawnDelegates(APawn* OwnerPawn);

	bool StartFight(AActor* NewEnemy); // called before delegate
};
