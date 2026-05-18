// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayTagManagerComp.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "GameFramework/PlayerState.h"
#include "multiplayerTEST/AbilitySystem/CoreAbility.h"
#include "Net/UnrealNetwork.h"

UGameplayTagManagerComp::UGameplayTagManagerComp()
{
	isDeadTag = FGameplayTag::RequestGameplayTag(FName("StatusEffect.RestrictiveEffects.Dead"));

	DefaultDeadTags.AddTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.RestrictiveEffects.Dead")));
}

void UGameplayTagManagerComp::BeginPlay()
{
	Super::BeginPlay();

	OverrideTags(DefaultAliveTags);

	if (GetOwner()->GetLocalRole() != ROLE_Authority) return;

	if (this == nullptr || GetOwner() == nullptr) {
		return;
	}
	APawn* OwnerPawn;

	if (APlayerState* PlayerStateOwner = Cast<APlayerState>(GetOwner())) {
		OwnerPawn = PlayerStateOwner->GetPawn();
	}
	else {
		OwnerPawn = Cast<APawn>(GetOwner());
	}

	if (!OwnerPawn) {
		return;
	}
	if (UHealthComponent* OwnerHealthComponent = OwnerPawn->GetComponentByClass<UHealthComponent>()) {
		OwnerHealthComponent->ActorDied.AddDynamic(this, &UGameplayTagManagerComp::ResetAfterDeath);
	}

}

void UGameplayTagManagerComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UGameplayTagManagerComp, PlayerTags, COND_SkipOwner);
}

void UGameplayTagManagerComp::ResetAfterDeath(AActor* DeadActor)
{
	OverrideTags(DefaultDeadTags);
}



void UGameplayTagManagerComp::AddTag(FGameplayTag TagToAdd)
{
	if (!this || PlayerTags.HasTag(isDeadTag)) { return; }

	PlayerTags.AddTag(TagToAdd);
}

void UGameplayTagManagerComp::RemoveTag(FGameplayTag TagToRemove)
{
	if (!this || PlayerTags.HasTag(isDeadTag)) { return; }

	PlayerTags.RemoveTag(TagToRemove);
}

void UGameplayTagManagerComp::AddTags(FGameplayTagContainer TagsToAdd)
{
	if (!this || PlayerTags.HasTag(isDeadTag)) { return; }

	PlayerTags.AppendTags(TagsToAdd);
}

void UGameplayTagManagerComp::RemoveTags(FGameplayTagContainer TagsToRemove)
{
	if (!this || PlayerTags.HasTag(isDeadTag)) { return; }

	PlayerTags.RemoveTags(TagsToRemove);
}

void UGameplayTagManagerComp::SetUpInitialTags()
{
	OverrideTags(DefaultAliveTags);
}

void UGameplayTagManagerComp::SetOwnerFactionTag(FGameplayTag NewOwnerFactionTag)
{
	OwnerFactionTag = NewOwnerFactionTag;
}

void UGameplayTagManagerComp::SetOwnerEnemiesFactionTags(FGameplayTagContainer NewOwnerEnemiesFactionTags)
{
	OwnerEnemiesFactionTags = NewOwnerEnemiesFactionTags;
}

void UGameplayTagManagerComp::OverrideTags(FGameplayTagContainer TagsToSet)
{
	PlayerTags = TagsToSet;
}
