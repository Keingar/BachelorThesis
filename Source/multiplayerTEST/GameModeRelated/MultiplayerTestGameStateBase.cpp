// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerTestGameStateBase.h"
#include "MultiplayerTEST/GameModeRelated/multiplayerTESTGameMode.h"
#include "GameFramework/GameState.h"
#include "MultiplayerTestPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "multiplayerTEST/Characters/multiplayerTESTCharacter.h"

AMultiplayerTestGameStateBase::AMultiplayerTestGameStateBase() {
	CurrentGamePhase = OpenWorld;
}

void AMultiplayerTestGameStateBase::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
	DOREPLIFETIME(AMultiplayerTestGameStateBase, CurrentGamePhase);
}

void AMultiplayerTestGameStateBase::SetCurrentGamePhase(GamePhase NewGamePhase)
{
	if (GetLocalRole() == ROLE_Authority) {
		CurrentGamePhase = NewGamePhase;
	}
}

void AMultiplayerTestGameStateBase::OnBossFightStarted(class AEnemyCore* CurrentBoss)
{
	SetCurrentGamePhase(BossFight);

	if (MyGameMode) { MyGameMode->AddActiveBoss(CurrentBoss); }

	BossFightStartedMulticast(CurrentBoss);
}

void AMultiplayerTestGameStateBase::BossFightStartedMulticast_Implementation(AEnemyCore* CurrentBoss)
{
	BossFightStarted.Broadcast(CurrentBoss);
}

void AMultiplayerTestGameStateBase::OnBossFightEnded(class AEnemyCore* CurrentBoss, bool BossFightResult)
{
	if (!MyGameMode || !MyGameMode->RemoveActiveBoss(CurrentBoss)) { return; }

	SetCurrentGamePhase(OpenWorld);
	BossFightEndedMulticast(CurrentBoss, BossFightResult);
}

void AMultiplayerTestGameStateBase::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	OnPlayerJoin.Broadcast(PlayerState);
}

void AMultiplayerTestGameStateBase::RemovePlayerState(APlayerState* PlayerState)
{
	OnSomeoneLeft(PlayerState);
	Super::RemovePlayerState(PlayerState);
}

void AMultiplayerTestGameStateBase::OnSomeoneSpawned(APlayerState* SpawnedPlayer)
{
	ActivePlayer++;
	Cast< AMultiplayerTestPlayerState>(SpawnedPlayer)->IsDead = false;
	OnPlayerSpawned.Broadcast(SpawnedPlayer);
}

void AMultiplayerTestGameStateBase::OnPlayerCharacterDied(APlayerState* DeeadPlayer)
{
	ActivePlayer--;
	PlayerDied.Broadcast(DeeadPlayer);
	if (!DeeadPlayer)return;

	Cast< AMultiplayerTestPlayerState>(DeeadPlayer)->IsDead = true;

}

void AMultiplayerTestGameStateBase::OnSomeoneLeft(APlayerState* PlayerThatLeft)
{

	if (Cast< AMultiplayerTestPlayerState>(PlayerThatLeft)->IsDead == false) {
		ActivePlayer--;
	}

	OnPlayerLeft.Broadcast(PlayerThatLeft);
}

void AMultiplayerTestGameStateBase::BeginPlay()
{
	Super::BeginPlay();
	MyGameMode = GetWorld()->GetAuthGameMode<AmultiplayerTESTGameMode>();
}

void AMultiplayerTestGameStateBase::BossFightEndedMulticast_Implementation(AEnemyCore* CurrentBoss, bool BossFightResult)
{
	BossFightEnded.Broadcast(CurrentBoss, BossFightResult);
}

