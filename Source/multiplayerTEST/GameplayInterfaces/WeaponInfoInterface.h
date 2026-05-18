// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "multiplayerTEST/CustomStructs/DamageInfo.h"
#include "multiplayerTEST/CustomEnums/ItemSubCategory.h"
#include "multiplayerTEST/CustomStructs/WeaponAttackInfo.h"
#include "WeaponInfoInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWeaponInfoInterface : public UInterface
{
	GENERATED_BODY()
};

class UCoreAbility;

class MULTIPLAYERTEST_API IWeaponInfoInterface
{
	GENERATED_BODY()
	
public:

	virtual const FWeaponAttackInfo& GetLightAttackInfo() const = 0;
	virtual const FWeaponAttackInfo& GetHeavyAttackInfo() const = 0;
	virtual const FWeaponAttackInfo& GetDoubleLightAttackInfo() const = 0;
	virtual const FWeaponAttackInfo& GetTH_LightAttackInfo() const = 0;
	virtual const FWeaponAttackInfo& GetRollAttackInfo() const = 0;
	
	virtual const FWeaponPatternInfo& GetFirstAttackAfterRoll() const = 0;
	virtual TSubclassOf<UCoreAbility> GetFirstAttackAfterRollAbilityClass() const = 0;

	virtual const FWeaponPatternInfo& GetFrontCritHitAttackInfo() const = 0;
	virtual TSubclassOf<UCoreAbility> GetFrontCritHitAbilityClass() const = 0;

	virtual EItemSubCategory GetWeaponCategory() const = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WeaponInfo")
	const FDamageInfo GetBaseWeaponDamage() const;
};