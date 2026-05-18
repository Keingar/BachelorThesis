// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreAbility.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/AttributeComponentInfo.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/Components/AbilitySystemComponent.h"

UCoreAbility::UCoreAbility()
{
	//AddToRoot();
}

UCoreAbility::~UCoreAbility()
{
}

bool UCoreAbility::TryUseAbilityStatic(TSubclassOf<class UCoreAbility> AbilityClass, float NewCost, TScriptInterface<IAttributeComponentInfo> AttributeComponentApplyCostTo, AActor* AvatarActorr, UGameplayTagManagerComp* TagsCompOfOwner, UObject* AdditionalDataa)
{
	if (!CanUseAbilityStatic(AbilityClass, AttributeComponentApplyCostTo, TagsCompOfOwner, AdditionalDataa)) {
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComp = AvatarActorr->GetComponentByClass<UAbilitySystemComponent>();

	UCoreAbility* NewAbility = NewObject<UCoreAbility>(AbilitySystemComp, AbilityClass);

	AbilitySystemComp->AddAbility(NewAbility);

	if (NewAbility->TryUseAbility(NewCost, AttributeComponentApplyCostTo, AvatarActorr, TagsCompOfOwner, AdditionalDataa)) {
		return true;
	} 

	AbilitySystemComp->RemoveAbility(NewAbility);

	return false;
}

bool UCoreAbility::CanUseAbility()
{
	if (!AttributeComponent || !TagManager) {
		return false;
	}

	const FGameplayTagContainer& OwnerTags = TagManager->GetTags();

	if(OwnerTags.HasAnyExact(ActivationBlockedTags)) {
		return false;
	}

	if (!OwnerTags.HasAllExact(ActivationRequiredTags)) {
		return false;
	}
	       
	for (const FGameplayTagContainer& Group : RequiredGroupsOftags)
	{
		if (OwnerTags.HasAny(Group))
		{
			if (!OwnerTags.HasAll(Group))
			{
				return false;
			}
		}
	}

	if (isUsingStamina) {
		return  (AttributeComponent->GetAttributeValue() > 0 ) ? true : false;
	}

	return (AttributeComponent->GetAttributeValue() - Cost >= 0) ? true : false;

}

bool UCoreAbility::CanUseAbilityStatic(TSubclassOf<class UCoreAbility> AbilityClass, TScriptInterface<IAttributeComponentInfo> AttrubuteComponentApplyCostTo, UGameplayTagManagerComp* TagsCompOfOwner, UObject* AdditionalDataa)
{
	if(!AbilityClass) {
		return false;
	}

	if (!AttrubuteComponentApplyCostTo || !TagsCompOfOwner) {
		return false;
	}

	const UCoreAbility* DefaultObj = AbilityClass->GetDefaultObject<UCoreAbility>();

	const FGameplayTagContainer& OwnerTags = TagsCompOfOwner->GetTags();

	if (OwnerTags.HasAnyExact(DefaultObj->ActivationBlockedTags)) {
		return false;
	}

	if (!OwnerTags.HasAllExact(DefaultObj->ActivationRequiredTags)) {
		return false;
	}

	for (const FGameplayTagContainer& Group : DefaultObj->RequiredGroupsOftags)
	{
		if (OwnerTags.HasAny(Group))
		{
			if (!OwnerTags.HasAll(Group))
			{
				return false; 
			}
		}
	}

	if (DefaultObj->isUsingStamina) {
		return  (AttrubuteComponentApplyCostTo->GetAttributeValue() > 0) ? true : false;
	}

	return (AttrubuteComponentApplyCostTo->GetAttributeValue() - DefaultObj->Cost >= 0) ? true : false;
}

void UCoreAbility::SetUpAbility(float NewCost, IAttributeComponentInfo* AttrubuteComponentApplyCostTo, AActor* AvatarActorr, UGameplayTagManagerComp* TagsCompOfOwner, UObject* AdditionalDataa)
{
	Cost = NewCost;
	AttributeComponent = AttrubuteComponentApplyCostTo;
	AvatarActor = AvatarActorr;
	TagManager = TagsCompOfOwner;
	AdditionalData = AdditionalDataa;
}

bool UCoreAbility::TryUseAbility(float NewCost, TScriptInterface<IAttributeComponentInfo> AttrubuteComponentApplyCostTo, AActor* AvatarActorr, UGameplayTagManagerComp* TagsCompOfOwner, UObject* AdditionalDataa)
{
	IAttributeComponentInfo* RawPointerToAttributeComponent = AttrubuteComponentApplyCostTo.GetInterface();

	SetUpAbility(NewCost, RawPointerToAttributeComponent, AvatarActorr, TagsCompOfOwner, AdditionalDataa);

	if (!CanUseAbility()) {
		return false;
	}

	ApplyTagsToOwner();
	//RemoveFromRoot();
	UseAbility();
	return true;

}

bool UCoreAbility::CommitAbility()
{
	if (!CanUseAbility()) {
		return false;
	}

	AttributeComponent->AddToAttribute(-Cost);
	return true;

}

void UCoreAbility::EndAbility()
{
	UAbilitySystemComponent* AbilitySystemComp = AvatarActor->GetComponentByClass<UAbilitySystemComponent>();
	AbilitySystemComp->RemoveAbility(this);

	this->ConditionalBeginDestroy();
}

void UCoreAbility::ApplyTagsToOwner()
{
	if (!TagManager) {
		return;
	}
}

void UCoreAbility::RemoveTagsFromOwner()
{
	if (!TagManager) {
		return;
	}
	TagManager->RemoveTags(TagsGrantedToOwner);
}

void UCoreAbility::BeginDestroy()
{
	//if (IsRooted()) {
	//	RemoveFromRoot();
//	}

	Super::BeginDestroy();
}