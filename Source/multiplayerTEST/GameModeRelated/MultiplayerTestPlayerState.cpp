// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerTestPlayerState.h"
#include "multiplayerTEST/Characters/multiplayerTESTCharacter.h"
#include "multiplayerTEST/Items/WearableItem.h"
#include "multiplayerTEST/Components/InventoryComponent.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTestGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "multiplayerTEST/Components/HealthComponent.h"

AMultiplayerTestPlayerState::AMultiplayerTestPlayerState() {

	Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");
	TagsManagerComponent = CreateDefaultSubobject<UGameplayTagManagerComp>("TagsManagerComp");
}

void AMultiplayerTestPlayerState::BeginPlay()
{
	Super::BeginPlay();
	IsDead = false;
	CustomGameState = GetWorld()->GetGameState<AMultiplayerTestGameStateBase>();

}

void AMultiplayerTestPlayerState::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiplayerTestPlayerState, CharacterName);
}

void AMultiplayerTestPlayerState::OnRep_CharacterName()
{
	OnUpdateCharacterName();
}

void AMultiplayerTestPlayerState::SetCharacterName(FText value)
{
	if (GetLocalRole() == ROLE_Authority) {
		CharacterName = value;
		OnUpdateCharacterName();
	}
}

void AMultiplayerTestPlayerState::ResetSoundClient_Implementation()
{
	StopSoundInBP();
}

void AMultiplayerTestPlayerState::ResetSound_Implementation()
{
	if (HasAuthority() && this != nullptr)
	{
		ResetSoundClient();
	}

}

UInventoryComponent* AMultiplayerTestPlayerState::GetInventory()
{
	return Inventory;
}

void AMultiplayerTestPlayerState::OnOwnerDies(AActor* DeadOwner)
{
	IsDead = true;
}