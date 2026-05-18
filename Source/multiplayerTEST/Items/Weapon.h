// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "multiplayerTEST/Items/WearableItem.h"
#include "multiplayerTEST/GameplayInterfaces/WeaponInfoInterface.h"
#include "multiplayerTEST/CustomStructs/HitBoxData.h"
#include "multiplayerTEST/CustomStructs/DamageInfo.h"
#include "multiplayerTEST/CustomStructs/WeaponPatternInfo.h"
#include "multiplayerTEST/CustomStructs/WeaponAttackInfo.h"
#include "Weapon.generated.h"

class UCoreAbility;

UCLASS()
class MULTIPLAYERTEST_API UWeapon : public UWearableItem, public IWeaponInfoInterface
{
	GENERATED_BODY()
public:
	UWeapon();

	virtual const FWeaponAttackInfo& GetLightAttackInfo() const override { return LightAttackInfo; }
	virtual const FWeaponAttackInfo& GetHeavyAttackInfo() const override { return HeavyAttackInfo; }
	virtual const FWeaponAttackInfo& GetDoubleLightAttackInfo() const override { return DoubleLightAttackInfo; }
	virtual const FWeaponAttackInfo& GetTH_LightAttackInfo() const override { return TH_LightAttackInfo; }
	virtual const FWeaponAttackInfo& GetRollAttackInfo() const override { return RollAttackInfo; }

	virtual const FWeaponPatternInfo& GetFirstAttackAfterRoll() const override { return FirstAttackAfterRoll; }
	virtual TSubclassOf<UCoreAbility> GetFirstAttackAfterRollAbilityClass() const override { return FirstAttackAfterRollAbilityClass; }

	virtual const FWeaponPatternInfo& GetFrontCritHitAttackInfo() const override { return FrontCritHitAttackInfo; }
	virtual TSubclassOf<UCoreAbility> GetFrontCritHitAbilityClass() const override { return FrontCritHitAbilityClass; }

	const FDamageInfo GetBaseWeaponDamage_Implementation() const override { return WeaponBaseDamage; }

	virtual EItemSubCategory GetWeaponCategory() const override { return ItemSubCategory; }


	virtual void Use() override;

	const TArray<FHitBoxData>* GetHitBoxes() const { return &HitBoxes; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool invertYawWhenEquipLH;

	UAnimMontage* GetBlockImpactMontage() { return BlockImpactMontage; }
	UAnimSequence* GetBlockLoopSequence() { return BlockLoopSequence; }
	UAnimSequence* GetBlockStartSequence() { return BlockStartSequence; }
	UAnimSequence* GetBlockEndSequence() { return BlockEndSequence; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAnimMontage* GetBlockBreakMontage() { return BlockBreakMontage; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetBlockAngleInDegrees() const { return BlockAngleInDegrees; }
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FHitBoxData> HitBoxes;

	UPROPERTY(EditDefaultsOnly)
	FDamageInfo WeaponBaseDamage;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* BlockImpactMontage;
	UPROPERTY(EditDefaultsOnly)
	UAnimSequence* BlockLoopSequence;
	UPROPERTY(EditDefaultsOnly)
	UAnimSequence* BlockStartSequence;
	UPROPERTY(EditDefaultsOnly)
	UAnimSequence* BlockEndSequence;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* BlockBreakMontage;

	UPROPERTY(EditDefaultsOnly)
	float BlockAngleInDegrees;

// light attack info
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAttackInfo")
	FWeaponAttackInfo LightAttackInfo;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAttackInfo")
	FWeaponAttackInfo HeavyAttackInfo;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAttackInfo")
	FWeaponAttackInfo DoubleLightAttackInfo;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAttackInfo")
	FWeaponAttackInfo TH_LightAttackInfo;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAttackInfo")
	FWeaponAttackInfo RollAttackInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAttackInfo")
	FWeaponPatternInfo FirstAttackAfterRoll;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAttackInfo")
	TSubclassOf<UCoreAbility>  FirstAttackAfterRollAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAttackInfo")
	FWeaponPatternInfo FrontCritHitAttackInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAttackInfo")
	TSubclassOf<UCoreAbility>  FrontCritHitAbilityClass;

};
