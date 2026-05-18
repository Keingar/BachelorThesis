#pragma once

#include "CoreMinimal.h"
#include "multiplayerTEST/CustomStructs/DamageInfo.h"
#include "multiplayerTEST/CustomStructs/WeaponAttackInfo.h"
#include "ModularAttackAbilityData.generated.h"

class UPlayerAttacksComponent;
class UImpactComponent;

UCLASS(BlueprintType)
class UModularAttackAbilityData : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly)
    FDamageInfo AttackDamageInfoRH;

    UPROPERTY(BlueprintReadOnly)
    FDamageInfo AttackDamageInfoLF;

    UPROPERTY(BlueprintReadOnly)
    FWeaponPatternInfo CurrentAttackInfo;

    UPROPERTY(BlueprintReadOnly)
	UPlayerAttacksComponent* PlayerAttacksComponent;

    UPROPERTY(BlueprintReadOnly)
    FWeaponAttackInfo currentAttacksEntireComboInfo;

    UPROPERTY(BlueprintReadOnly)
    UImpactComponent* ImpactOfTargetActor; // used only for crit attacks

    // Factory functions
    static UModularAttackAbilityData* CreateRH(UObject* Outer, const FDamageInfo& RHDamage, FWeaponPatternInfo AttackInfo, UPlayerAttacksComponent* AttackComponent, FWeaponAttackInfo EntireComboInfo)
    {
        UModularAttackAbilityData* Obj = NewObject<UModularAttackAbilityData>(Outer);
        Obj->AttackDamageInfoRH = RHDamage;
        Obj->CurrentAttackInfo = AttackInfo;
        Obj->PlayerAttacksComponent = AttackComponent;
        Obj->currentAttacksEntireComboInfo = EntireComboInfo;

        return Obj;
    }

    static UModularAttackAbilityData* CreateRHForStaggerAttack(UObject* Outer, const FDamageInfo& RHDamage, FWeaponPatternInfo AttackInfo, UPlayerAttacksComponent* AttackComponent, UImpactComponent* ActorToAttack)
    {
        UModularAttackAbilityData* Obj = NewObject<UModularAttackAbilityData>(Outer);
        Obj->AttackDamageInfoRH = RHDamage;
        Obj->CurrentAttackInfo = AttackInfo;
        Obj->PlayerAttacksComponent = AttackComponent;
		Obj->ImpactOfTargetActor = ActorToAttack;

        return Obj;
    }

    static UModularAttackAbilityData* CreateLH(UObject* Outer, const FDamageInfo& LHDamage, FWeaponPatternInfo AttackInfo, UPlayerAttacksComponent* AttackComponent, FWeaponAttackInfo EntireComboInfo)
    {
        UModularAttackAbilityData* Obj = NewObject<UModularAttackAbilityData>(Outer);
        Obj->AttackDamageInfoLF = LHDamage;
        Obj->CurrentAttackInfo = AttackInfo;
        Obj->PlayerAttacksComponent = AttackComponent;
        Obj->currentAttacksEntireComboInfo = EntireComboInfo;

        return Obj;
    }

    static UModularAttackAbilityData* CreateBoth(UObject* Outer, const FDamageInfo& RHDamage, const FDamageInfo& LHDamage, FWeaponPatternInfo AttackInfo, UPlayerAttacksComponent* AttackComponent, FWeaponAttackInfo EntireComboInfo)
    {
        UModularAttackAbilityData* Obj = NewObject<UModularAttackAbilityData>(Outer);
        Obj->AttackDamageInfoRH = RHDamage;
        Obj->AttackDamageInfoLF = LHDamage;
        Obj->CurrentAttackInfo = AttackInfo;
        Obj->PlayerAttacksComponent = AttackComponent;
        Obj->currentAttacksEntireComboInfo = EntireComboInfo;

        return Obj;
    }
};
