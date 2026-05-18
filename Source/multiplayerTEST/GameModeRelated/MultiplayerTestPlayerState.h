// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "MultiplayerTestPlayerState.generated.h"

class AMultiplayerTestGameStateBase;
class UInventoryComponent;
class UGameplayTagManagerComp;

UCLASS()
class MULTIPLAYERTEST_API AMultiplayerTestPlayerState : public APlayerState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	UGameplayTagManagerComp* TagsManagerComponent;

protected:

	virtual void BeginPlay() override;

	AMultiplayerTestGameStateBase* CustomGameState;
	void OnOwnerDies(AActor* DeadOwner);
public:
	AMultiplayerTestPlayerState();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "ReplicatedStuff")
	FORCEINLINE FText GetCharacterName() const { return CharacterName; }

	UFUNCTION(BlueprintCallable, Category = "ReplicatedStuff")
	void SetCharacterName(FText value);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void ResetSound();

	UFUNCTION(Client, Reliable)
	void ResetSoundClient();

	UFUNCTION(BlueprintImplementableEvent)
	void StopSoundInBP();

	UInventoryComponent* GetInventory();
	UGameplayTagManagerComp* GetTagManager() { return TagsManagerComponent; }
	bool IsDead;
public:
	bool bHasSpawned = false;
protected:
	
	UFUNCTION()
	void OnRep_CharacterName();

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateCharacterName();


protected:

	UPROPERTY(ReplicatedUsing = OnRep_CharacterName)
	FText CharacterName;
};
