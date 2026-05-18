// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAttacksComponent.h"
#include "multiplayerTEST/AbilitySystem/CoreAbility.h"
#include "multiplayerTEST/CustomStructs/StructsForAbilities/ModularAttackAbilityData.h"
#include "multiplayerTEST/GameplayInterfaces/WeaponInfoInterface.h"
#include "multiplayerTEST/Characters/multiplayerTESTCharacter.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/Components/StaminaComponent.h"
#include "multiplayerTEST/Items/WearableItem.h"
#include "multiplayerTEST/Components/HitDetectionComponent.h"
#include "multiplayerTEST/Components/ImpactComponent.h"

void UPlayerAttacksComponent::BeginPlay() {
	Super::BeginPlay();

	OwnerCharacter = Cast<AmultiplayerTESTCharacter>(GetOwner());

	OwnerHitDetectionComp = Cast<UHitDetectionComponent>(OwnerCharacter->GetComponentByClass<UHitDetectionComponent>());
}

void UPlayerAttacksComponent::HeavyAttackServer_Implementation()
{
	TryHeavyAttack();
}

bool UPlayerAttacksComponent::TryHeavyAttack()
{
	if (!OwnerCharacter) return false;

	IWeaponInfoInterface* CurrentWeapon = Cast<IWeaponInfoInterface>(IPlayerInformationInterface::Execute_GetWeaponRH1(OwnerCharacter));

	if (!CurrentWeapon) return false;

	FWeaponAttackInfo RHHeavyAttackInfo = CurrentWeapon->GetHeavyAttackInfo();
	int correctCombo = CheckIfSameAttack == 1 ? Combo : 0;

	UModularAttackAbilityData* AttackData = UModularAttackAbilityData::CreateRH(this, CurrentWeapon->GetBaseWeaponDamage_Implementation(), GetCurrentAttackPattern(RHHeavyAttackInfo, correctCombo), this, RHHeavyAttackInfo);

	if (!UCoreAbility::CanUseAbilityStatic(RHHeavyAttackInfo.AbilityClass, OwnerCharacter->GetStaminaComponent(), OwnerTagManager, AttackData)) return true;

	if (OwnerCharacter->GetLocalRole() != ROLE_Authority) {
		HeavyAttackServer();
	}

	if (CheckIfSameAttack != 1) ResetCombo();
	CheckIfSameAttack = 1;

	bool bSuccessfulUse = UCoreAbility::TryUseAbilityStatic(
		RHHeavyAttackInfo.AbilityClass,
		0,
		OwnerCharacter->GetStaminaComponent(),
		OwnerCharacter,
		OwnerTagManager,
		AttackData
	);

	return !bSuccessfulUse;
}

bool UPlayerAttacksComponent::TryLightAttack()
{
	if (!OwnerCharacter) return false;

	IWeaponInfoInterface* CurrentWeapon = Cast<IWeaponInfoInterface>(IPlayerInformationInterface::Execute_GetWeaponRH1(OwnerCharacter));

	if (!CurrentWeapon || !OwnerTagManager) return false;

	FWeaponAttackInfo CorrectAttackInfo; // can be light attack or two hand light attack
	int correctSameAttacktoCheck = 2;

	// priorities:
	// 1. Crit hit after enemy stagger
	// 2. first roll attack
	// 3. roll attack
	// 4. two hand light attack
	// 5. light attack

	UImpactComponent* ActorToStaggerAttack = GetActorToStagger();

	if (ActorToStaggerAttack) {
		return TryStaggerAttackEnemy(ActorToStaggerAttack);
	}
	else if (OwnerTagManager->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.ComboAnimStatus.CanRollAgain")))
		&& OwnerTagManager->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.RestrictiveEffects.Rolling"))))
	{
		return TryFirstRollAttack();
	}
	else if (!CurrentWeapon->GetRollAttackInfo().WeaponPatterns.IsEmpty()
		&& OwnerTagManager->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.RestrictiveEffects.Attacking.RollAttack")))
		&& OwnerTagManager->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.ComboAnimStatus.CanRollAttackAgain"))))
	{
		CorrectAttackInfo = CurrentWeapon->GetRollAttackInfo();
		correctSameAttacktoCheck = 5;
	}
	else if (OwnerTagManager->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.UnrestrictiveEffects.TwoHandingWeapon")))) {
		CorrectAttackInfo = CurrentWeapon->GetTH_LightAttackInfo();
		correctSameAttacktoCheck = 4;
	}
	else {
		CorrectAttackInfo = CurrentWeapon->GetLightAttackInfo();
	}

	int correctCombo = CheckIfSameAttack == correctSameAttacktoCheck ? Combo : 0;

	UModularAttackAbilityData* AttackData = UModularAttackAbilityData::CreateRH(this, CurrentWeapon->GetBaseWeaponDamage_Implementation(), GetCurrentAttackPattern(CorrectAttackInfo, correctCombo), this, CorrectAttackInfo);

	if (!UCoreAbility::CanUseAbilityStatic(CorrectAttackInfo.AbilityClass, OwnerCharacter->GetStaminaComponent(), OwnerTagManager, AttackData)) return true;

	if (OwnerCharacter->GetLocalRole() != ROLE_Authority) {
		LightAttackServer();
	}

	if (CheckIfSameAttack != correctSameAttacktoCheck) ResetCombo();
	CheckIfSameAttack = correctSameAttacktoCheck;

	bool bSuccessfulUse = UCoreAbility::TryUseAbilityStatic(
		CorrectAttackInfo.AbilityClass,
		0,
		OwnerCharacter->GetStaminaComponent(),
		OwnerCharacter,
		OwnerTagManager,
		AttackData
	);

	return !bSuccessfulUse;
}

UImpactComponent* UPlayerAttacksComponent::GetActorToStagger() {
	TArray<AActor*> HitActors = OwnerHitDetectionComp->GetActorsInFrontOfOwner();

	if (HitActors.IsEmpty()) return nullptr;

	for (AActor* HitActor : HitActors)
	{
		UImpactComponent* HitActorImpactComp = Cast<UImpactComponent>(HitActor->GetComponentByClass<UImpactComponent>());

		if (!HitActorImpactComp) continue;

		if (HitActorImpactComp->bEnemyCanStaggerHitCheck(GetOwner())) {
			return HitActorImpactComp;
		}
	}

	return nullptr;
}

bool UPlayerAttacksComponent::TryStaggerAttackEnemy(UImpactComponent* ActorToStaggerAttack) {
	IWeaponInfoInterface* CurrentWeapon = Cast<IWeaponInfoInterface>(IPlayerInformationInterface::Execute_GetWeaponRH1(OwnerCharacter));

	if (!CurrentWeapon || !OwnerTagManager) return false;

	TSubclassOf<class UCoreAbility> RollFirstAttackAbilityClass = CurrentWeapon->GetFrontCritHitAbilityClass();
	if (!RollFirstAttackAbilityClass) return false;
	FWeaponPatternInfo CorrectAttackInfo = CurrentWeapon->GetFrontCritHitAttackInfo(); // can be light attack or two hand light attack

	UModularAttackAbilityData* AttackData = UModularAttackAbilityData::CreateRHForStaggerAttack(this, CurrentWeapon->GetBaseWeaponDamage_Implementation(), CorrectAttackInfo, this, ActorToStaggerAttack);

	if (!UCoreAbility::CanUseAbilityStatic(RollFirstAttackAbilityClass, OwnerCharacter->GetStaminaComponent(), OwnerTagManager, AttackData)) return true;

	if (OwnerCharacter->GetLocalRole() != ROLE_Authority) {
		TryStaggerAttackEnemyServer(ActorToStaggerAttack);
	}

	bool bSuccessfulUse = UCoreAbility::TryUseAbilityStatic(
		RollFirstAttackAbilityClass,
		0,
		OwnerCharacter->GetStaminaComponent(),
		OwnerCharacter,
		OwnerTagManager,
		AttackData
	);

	return !bSuccessfulUse;
}

bool UPlayerAttacksComponent::TryFirstRollAttack()
{
	IWeaponInfoInterface* CurrentWeapon = Cast<IWeaponInfoInterface>(IPlayerInformationInterface::Execute_GetWeaponRH1(OwnerCharacter));

	if (!CurrentWeapon || !OwnerTagManager) return false;

	TSubclassOf<class UCoreAbility> RollFirstAttackAbilityClass = CurrentWeapon->GetFirstAttackAfterRollAbilityClass();
	if (!RollFirstAttackAbilityClass) return false;
	FWeaponPatternInfo CorrectAttackInfo = CurrentWeapon->GetFirstAttackAfterRoll(); // can be light attack or two hand light attack

	FWeaponAttackInfo RHHeavyAttackInfo = CurrentWeapon->GetRollAttackInfo();

	UModularAttackAbilityData* AttackData = UModularAttackAbilityData::CreateRH(this, CurrentWeapon->GetBaseWeaponDamage_Implementation(), CorrectAttackInfo, this, RHHeavyAttackInfo);

	if (!UCoreAbility::CanUseAbilityStatic(RollFirstAttackAbilityClass, OwnerCharacter->GetStaminaComponent(), OwnerTagManager, AttackData)) return true;

	if (OwnerCharacter->GetLocalRole() != ROLE_Authority) {
		TryfirstRollAttackServer();
	}

	bool bSuccessfulUse = UCoreAbility::TryUseAbilityStatic(
		RollFirstAttackAbilityClass,
		0,
		OwnerCharacter->GetStaminaComponent(),
		OwnerCharacter,
		OwnerTagManager,
		AttackData
	);

	return !bSuccessfulUse;
}

void UPlayerAttacksComponent::TryfirstRollAttackServer_Implementation()
{
	TryFirstRollAttack();
}

void UPlayerAttacksComponent::LightAttackServer_Implementation()
{
	TryLightAttack();
}

void UPlayerAttacksComponent::TryDoubleLightAttackServer_Implementation()
{
	TryDoubleLightAttack();
}

void UPlayerAttacksComponent::TryStaggerAttackEnemyServer_Implementation(UImpactComponent* ActorToStaggerAttack)
{
	if (ActorToStaggerAttack && ActorToStaggerAttack->bEnemyCanStaggerHitCheck(OwnerCharacter)) {
		TryStaggerAttackEnemy(ActorToStaggerAttack);
	}
}

bool UPlayerAttacksComponent::TryDoubleLightAttackOrBlock() {
	if (!OwnerCharacter) return false;

	if (!OwnerTagManager) return false;

	IWeaponInfoInterface* CurrentRHWeapon = Cast<IWeaponInfoInterface>(IPlayerInformationInterface::Execute_GetWeaponRH1(OwnerCharacter));

	if (!CurrentRHWeapon) return false;

	if (OwnerTagManager->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.UnrestrictiveEffects.TwoHandingWeapon")))) {
		TryBlock();
		return false;
	}

	IWeaponInfoInterface* CurrentLHWeapon = Cast<IWeaponInfoInterface>(IPlayerInformationInterface::Execute_GetWeaponLH1(OwnerCharacter));

	if (!CurrentLHWeapon) return false;

	if (CurrentLHWeapon->GetWeaponCategory() == EItemSubCategory::Shield) {
		TryBlock();
		return false;
	}

	return TryDoubleLightAttack();
}

void UPlayerAttacksComponent::TryBlock()
{
	OwnerCharacter->OnStartBlock();
}


bool UPlayerAttacksComponent::TryDoubleLightAttack()
{
	IWeaponInfoInterface* CurrentRHWeapon = Cast<IWeaponInfoInterface>(IPlayerInformationInterface::Execute_GetWeaponRH1(OwnerCharacter));

	if (!CurrentRHWeapon) return false;

	IWeaponInfoInterface* CurrentLHWeapon = Cast<IWeaponInfoInterface>(IPlayerInformationInterface::Execute_GetWeaponLH1(OwnerCharacter));

	if (!CurrentLHWeapon) return false;

	if (CurrentLHWeapon->GetWeaponCategory() != CurrentRHWeapon->GetWeaponCategory()) return false;

	FWeaponAttackInfo DoubleLightAttackInfo = CurrentRHWeapon->GetDoubleLightAttackInfo();

	int correctCombo = CheckIfSameAttack == 3 ? Combo : 0;

	UModularAttackAbilityData* AttackData = UModularAttackAbilityData::CreateBoth(this, CurrentRHWeapon->GetBaseWeaponDamage_Implementation(), CurrentLHWeapon->GetBaseWeaponDamage_Implementation(), GetCurrentAttackPattern(DoubleLightAttackInfo, correctCombo), this, DoubleLightAttackInfo);

	if (!UCoreAbility::CanUseAbilityStatic(DoubleLightAttackInfo.AbilityClass, OwnerCharacter->GetStaminaComponent(), OwnerTagManager, AttackData)) return true;

	if (OwnerCharacter->GetLocalRole() != ROLE_Authority) {
		TryDoubleLightAttack();
	}

	if (CheckIfSameAttack != 3) ResetCombo();
	CheckIfSameAttack = 3;

	bool bSuccessfulUse = UCoreAbility::TryUseAbilityStatic(
		DoubleLightAttackInfo.AbilityClass,
		0,
		OwnerCharacter->GetStaminaComponent(),
		OwnerCharacter,
		OwnerTagManager,
		AttackData
	);

	return !bSuccessfulUse;
}

// combo functionality
FWeaponPatternInfo UPlayerAttacksComponent::GetCurrentAttackPattern(FWeaponAttackInfo WeaponAttackInfo, int ComboIndex) const
{
	return WeaponAttackInfo.WeaponPatterns[WeaponAttackInfo.attackAnimationSequence[ComboIndex]];
}

void UPlayerAttacksComponent::IncreaseCombo()
{

	if (Combo == CurrentAttackInfo.attackAnimationSequence.Num() - 1)
	{
		Combo = 0;

		return;
	}

	Combo++;
}

void UPlayerAttacksComponent::ResetCombo()
{
	Combo = 0;
}

int UPlayerAttacksComponent::GetCombo()
{
	return Combo;
}

void UPlayerAttacksComponent::SetCurrentAttackInfo(FWeaponAttackInfo newAttackInfo)
{
	CurrentAttackInfo = newAttackInfo;
}
