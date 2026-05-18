// Fill out your copyright notice in the Description page of Project Settings.


#include "RollComponent.h"
#include "multiplayerTEST/AbilitySystem/CoreAbility.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/TagManagerOwner.h"
#include "multiplayerTEST/Components/StaminaComponent.h"
#include "multiplayerTEST/AbilitySystem/RandomUObjects/RollBullshitData.h"

URollComponent::URollComponent() {
	StaminaRequiredForRoll = 20.f;
}

void URollComponent::BeginPlay()
{
	Super::BeginPlay();

	StaminaComponent = TScriptInterface<IAttributeComponentInfo>(GetOwner()->GetComponentByClass<UStaminaComponent>());
	SetUpTagManager();
}

bool URollComponent::TryRoll(double XAxis, double YAxis)
{
	if (!RollAbilityClass) return false;

	if (UCoreAbility::CanUseAbilityStatic(RollAbilityClass, StaminaComponent, TagManager, nullptr)) {
		if (GetOwner()->GetLocalRole() != ROLE_Authority) {
			TryRollAbilityOnServer(XAxis, YAxis);
		}

		BullshitData = NewObject<URollBullshitData>();
		BullshitData->XThingy = XAxis;
		BullshitData->YThingy = YAxis;
		return !UCoreAbility::TryUseAbilityStatic(RollAbilityClass, StaminaRequiredForRoll, StaminaComponent, GetOwner(), TagManager, BullshitData);
	}
	return true;
}

void URollComponent::TryRollAbilityOnServer_Implementation(double XAxis, double YAxis) {
	TryRoll(XAxis, YAxis);
}

void URollComponent::SetUpTagManager()
{
	TagManager = Cast<ITagManagerOwner>(GetOwner())->GetTagManager_Implementation();

	FTimerHandle SetUpTagManagerTimerHandle;
	if (!TagManager) {
		GetWorld()->GetTimerManager().SetTimer(SetUpTagManagerTimerHandle, this, &URollComponent::SetUpTagManager, 0.1, false);
	}
}