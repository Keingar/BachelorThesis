// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "multiplayerTEST/CustomEnums/GamePhase.h"
#include "MultiplayerTestGameStateBase.generated.h"

class AEnemyCore;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBossFightStarted, class AEnemyCore*, CurrentBoss);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBossFightEnded, class AEnemyCore*, CurrentBoss, bool, isBossDefeated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoin, class APlayerState*, NewPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeft, class APlayerState*, PlayerThatLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerSpawned, class APlayerState*, SpawnedPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerCharacterDied, class APlayerState*, DeadPlayer);


UCLASS()
class MULTIPLAYERTEST_API AMultiplayerTestGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMultiplayerTestGameStateBase();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "GameModeSettings")
	FORCEINLINE GamePhase GetCurrentGamePhase() const { return CurrentGamePhase; }

	UFUNCTION(BlueprintCallable)
	void SetCurrentGamePhase(GamePhase NewGamePhase);

	UFUNCTION(BlueprintCallable)
	void OnBossFightStarted(AEnemyCore* CurrentBoss);

	UFUNCTION(BlueprintCallable)
	void OnBossFightEnded(AEnemyCore* CurrentBoss, bool BossFightResult);

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	void OnSomeoneSpawned(APlayerState* SpawnedPlayer);
	void OnPlayerCharacterDied(APlayerState* DeeadPlayer);
	void OnSomeoneLeft(APlayerState* PlayerThatLeft);

	UFUNCTION(BlueprintCallable)
	int GetActivePlayerCount() const { return ActivePlayer; }
public:

	UPROPERTY(BlueprintAssignable)
	FBossFightStarted BossFightStarted;

	UPROPERTY(BlueprintAssignable)
	FBossFightEnded BossFightEnded;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerLeft OnPlayerLeft;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerJoin OnPlayerJoin;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerSpawned OnPlayerSpawned;

	UPROPERTY(BlueprintAssignable)
	FPlayerCharacterDied PlayerDied;

protected:

//	UPROPERTY(Category="CustomGameStateSettings")
	UPROPERTY(Replicated)
	TEnumAsByte<GamePhase> CurrentGamePhase;

	class AmultiplayerTESTGameMode* MyGameMode;

	void BeginPlay()override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "GameRules")
	int ActivePlayer;

	UFUNCTION(NetMulticast, Reliable)
	void BossFightStartedMulticast(AEnemyCore* CurrentBoss);

	UFUNCTION(NetMulticast, Reliable)
	void BossFightEndedMulticast(AEnemyCore* CurrentBoss, bool BossFightResult);
};
