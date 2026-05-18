// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "multiplayerTEST/CustomStructs/WeaponAttackInfo.h"
#include "PlayerAttacksComponent.generated.h"

class UImpactComponent;
class AmultiplayerTESTCharacter;
class UHitDetectionComponent;
class UGameplayTagManagerComp;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTEST_API UPlayerAttacksComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	// return true if should be used for LastInputAction
	bool TryLightAttack();
	// return true if should be used for LastInputAction
	bool TryHeavyAttack();
	// return true if should be used for LastInputAction
	bool TryDoubleLightAttackOrBlock();

	//  return true if should be used for LastInputAction
	// is set to public to use for LastInputAction in player character
	bool TryDoubleLightAttack();

	UPROPERTY()
	UGameplayTagManagerComp* OwnerTagManager;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FWeaponAttackInfo GetCurrentAttackInfo() const { return CurrentAttackInfo; }
protected:
	UFUNCTION(Server, Reliable)
	void LightAttackServer();
	UFUNCTION(Server, Reliable)
	void HeavyAttackServer();
	UFUNCTION(Server, Reliable)
	void TryDoubleLightAttackServer();

	UFUNCTION(Server, Reliable)
	void TryfirstRollAttackServer();

	UFUNCTION(Server, Reliable)
	void TryStaggerAttackEnemyServer(UImpactComponent* ActorToStaggerAttack);

	void BeginPlay() override;

	void TryBlock();

	bool TryFirstRollAttack();

	UImpactComponent* GetActorToStagger();
	bool TryStaggerAttackEnemy(UImpactComponent* ActorToStaggerAttack);

	// combo index is parameter for situations when differnet type of attack is starting so we can pass 0 manually without actually changing combo in case it fails
	FWeaponPatternInfo GetCurrentAttackPattern(FWeaponAttackInfo WeaponAttackInfo, int ComboIndex)const;
protected:
	int CheckIfSameAttack = -1;

	int Combo;

	UPROPERTY()
	AmultiplayerTESTCharacter* OwnerCharacter;
	UPROPERTY()
	UHitDetectionComponent* OwnerHitDetectionComp;

	FWeaponAttackInfo CurrentAttackInfo;
public: // public to let abilities modify combo
	// combo functionality
	UFUNCTION(BlueprintCallable)
	void IncreaseCombo();

	UFUNCTION(BlueprintCallable)
	void ResetCombo();

	UFUNCTION(BlueprintCallable)
	int GetCombo();

	UFUNCTION(BlueprintCallable)
	void SetCurrentAttackInfo(FWeaponAttackInfo newAttackInfo);
};
