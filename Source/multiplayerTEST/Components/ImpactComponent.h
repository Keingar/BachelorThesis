// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "multiplayerTEST/CustomStructs/DamageInfo.h"
#include "multiplayerTEST/CustomStructs/BlockInfo.h"
#include "GameplayTagContainer.h"
#include "ImpactComponent.generated.h"

class UGameplayTagManagerComp;
class UHealthComponent;
class UStaminaComponent;
class UMaterialInterface;
class UCapsuleComponent;
class ACoreAIController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGotHitFromDirection);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGotHit, AActor*, InstigatorActor, float, AngleDegrees);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStaggerStatusChange);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTEST_API UImpactComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UImpactComponent();

	void GotHit(const FHitResult& DetectedHit, AActor* InstigatorActorr, FDamageInfo HitDamage);

	bool bEnemyCanStaggerHitCheck(AActor* InstigatorActor);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector GetStaggerCritHitLocation() { return OnStaggerCritHitEnemyTargetLocation;}
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FRotator GetStaggerCritHitRotation() { return OnStaggerCritHitEnemyTargetRotation; }
	FVector GetStaggerCritHitBoxCenter() { return StaggerCritHitBoxCenter; }
	float GetStaggerCritHitBoxRadius() { return StaggerCritHitBoxRadius; }
	UMaterialInterface* GetDecalMaterialForStaggerZone() { return DecalMaterialForStaggerZone; }

	void ResetImpactComponent(); 

	UPROPERTY(BlueprintAssignable)
	FGotHitFromDirection OnGotHitFromRight;

	UPROPERTY(BlueprintAssignable)
	FGotHitFromDirection OnGotHitFromLeft;

	UPROPERTY(BlueprintAssignable)
	FGotHitFromDirection OnGotHitFromStraight;

	UPROPERTY(BlueprintAssignable)
	FGotHitFromDirection OnGotHitFromTop;

	UPROPERTY(BlueprintAssignable)
	FGotHitFromDirection OnGotBlockedHit;

	UPROPERTY(BlueprintAssignable)
	FGotHit OnGotHit;

	UPROPERTY(BlueprintAssignable)
	FStaggerStatusChange OnStaggerStart;

	UPROPERTY(BlueprintAssignable)
	FStaggerStatusChange OnBlockBreak;

	void SetOwnerGhostAnimInstance(UAnimInstance* NewGhostAnimInstance) {
		OwnerGhostAnimInstance = NewGhostAnimInstance;
	}

protected:
	virtual void BeginPlay() override;

	void SelectAndPlayImpactAnimation(const FHitResult& Hit, AActor* InstigatorActor, bool bBlockedAttack);

	UGameplayTagManagerComp* OwnerTagComp;

	void SetUpOwnerTagManagerRef();

	bool isBlocking(AActor* ActorToCheck, float blockAngleToCheck);

	void getCurrentBlockInfo(float* blockAngleInDegrees, FBlockInfo& InBlockInfo);

	void PlayMontageForOwnerAndItsGhost(UAnimMontage* MontageToPlay);
protected:
	UAnimInstance* OwnerAnimInstance;
	UAnimInstance* OwnerGhostAnimInstance;

	UStaminaComponent* OwnerStaminaComponent; // not every owner have it
	UHealthComponent* OwnerHealthComponent;
	AActor* OwnerCharacter;
	UCapsuleComponent* OwnerCapsule;
	ACoreAIController* OwnerAIController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float TopHitThreshold;

	// poise stuff
	// returns true if poise reach 0
	bool TakePoiseDamage(float PoiseDamage);

	UPROPERTY(EditAnywhere) 
	int32 maxPoise;
	int32 currentPoise;

	UPROPERTY(EditDefaultsOnly)
	bool bCanBeStaggered;

	struct FGameplayTag StaggerTag; 

	UPROPERTY(EditDefaultsOnly, Category = "StaggerCrit")
	UAnimMontage* StaggerCritHitMontage;

	// where player will be on stagger crit hit
	UPROPERTY(EditDefaultsOnly, Category = "StaggerCrit")
	FVector OnStaggerCritHitEnemyTargetLocation;
	UPROPERTY(EditDefaultsOnly, Category = "StaggerCrit")
	FRotator OnStaggerCritHitEnemyTargetRotation;

	// where player should be to crit hit on stagger
	UPROPERTY(EditDefaultsOnly, Category = "StaggerCrit")
	FVector StaggerCritHitBoxCenter;
	UPROPERTY(EditDefaultsOnly, Category = "StaggerCrit")
	float StaggerCritHitBoxRadius = 50;
	UPROPERTY(EditDefaultsOnly)
	float AcceptableAngle = 60;
	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* DecalMaterialForStaggerZone;

	UPROPERTY(EditDefaultsOnly)
	bool bPlayImpactAnimationOnDeath = false;
};
