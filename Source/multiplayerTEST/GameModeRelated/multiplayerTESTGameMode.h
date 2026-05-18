// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "multiplayerTEST/CustomEnums/GamePhase.h"
#include "multiplayerTESTGameMode.generated.h"

class AEnemyCore;
class ASpawnPoint;
class AMultiplayerTestGameStateBase;
class USaveGameSubsystem;
class URegistrySubsystem;

UCLASS(minimalapi)
class AmultiplayerTESTGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AmultiplayerTESTGameMode();

	void Respawn(AController* Controller);
	
	UFUNCTION()
	void Spawn(AController* Controller);

	UFUNCTION(BlueprintPure, Category = "GameModeSettings")
	FORCEINLINE TArray<AEnemyCore*> GetCurrentBosses() const { return ListOfCurrentBosses; }

	UFUNCTION(BlueprintCallable, Category = "GameModeSettings")
	void AddActiveBoss(AEnemyCore* NewBoss);

	UFUNCTION(BlueprintCallable, Category = "GameModeSettings")
	bool RemoveActiveBoss(AEnemyCore* BossToRemove);

	UFUNCTION(BlueprintCallable) // called for bonfire reset
	void ResetPlayerCharacters();

	UFUNCTION(BlueprintCallable, Category = "Bonpile")
	void TeleportAllPlayersToBonpile(FName BonpileID);


public:

	UPROPERTY(BlueprintReadOnly, Category = "GameRules")
	int ActivePlayer;
	virtual void RestartPlayer(AController* NewPlayer) override;

protected:

	virtual void BeginPlay() override;

	void RespawnAllPlayers();
	
	virtual void PostLogin(APlayerController* NewPlayer)override;

	virtual void Logout(AController* Exiting)override;

	virtual AActor* ChoosePlayerStart_Implementation(AController *Player)override;

	void CheckIfLostDuringBossFight();

	UPROPERTY(EditAnywhere, Category = "GameRules")
	FRotator DefaultSpawnRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "GameRules")
	float RespawnTimer;

	float TimeBeforeResettingAfterFinishRound;
	TArray<AEnemyCore*> ListOfCurrentBosses;

	UPROPERTY(BlueprintReadOnly)
	AMultiplayerTestGameStateBase* CustomGameState;

private:
	void CacheSubsystems();
	void DetermineSpawnLocation(AController* Controller, bool bIsFirstSpawn, FVector& OutLocation, FRotator& OutRotation);
	bool GetFirstSpawnLocation(FVector& OutLocation, FRotator& OutRotator);
	bool GetLocationOfSavedBonpile(FVector& OutLocation, FRotator& OutRotation);
	void GetDefaultSpawnLocation(FVector& OutLocation, FRotator& OutRotation);

	// Delay (seconds) before teleporting the very first spawn for a player (e.g., let moving platforms settle)
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	float FirstSpawnDelay = 1;

	// cached subsystems
	UPROPERTY()
	USaveGameSubsystem* SaveGameSubsystem = nullptr;

	UPROPERTY()
	URegistrySubsystem* RegistrySubsystem = nullptr;
};