// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/ControllerInfo.h"
#include "MyPlayerController.generated.h"

class AMultiplayerTeestHUD;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFightStartPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFightStopPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewSpectate, APlayerState*, CurrentSpectatedPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBossFight, AActor*, EnemyBoss);


UCLASS()
class MULTIPLAYERTEST_API AMyPlayerController : public APlayerController, public IControllerInfo
{
	GENERATED_BODY()

public:
	AMyPlayerController();
	UFUNCTION(BlueprintCallable)
	void SpectateNextPawn();
	UFUNCTION(BlueprintCallable)
	void SpectatePreviousPawn();

	void ChooseSpectatedPawn();

	AMultiplayerTeestHUD* GetCustomHUD() const;

	void ResetController();
public:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	bool isOpenInventory;

	int CurrentSpectatedPawn;

	bool OnFightStart(AActor* newEnemy);

	UPROPERTY(BlueprintAssignable)
	FFightStartPlayer OnFightStarted;

	UPROPERTY(BlueprintAssignable)
	FFightStopPlayer OnFigthStopped;

	UPROPERTY(BlueprintAssignable)
	FNewSpectate OnSpectatedSwitched;

	UPROPERTY(BlueprintAssignable)
	FBossFight OnBossFightStarted;

	UPROPERTY(BlueprintAssignable)
	FBossFight OnBossFightStopped;

	virtual bool const GetIsOpenInventory_Implementation() override { return isOpenInventory; }

	virtual void SetIsOpenInventory_Implementation(bool newVal) override;

	void CreateHUD();

	void OnOwnerDied();

	void SwitchToSpectatePawn();

	void DestroyHUD();
	void ResetHUDAfterBonfireRest();
	
	AMultiplayerTeestHUD* CustomHUD;

protected:
	TArray<AActor*> ListOfEnemies;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTargetDied(AActor* DeadEnemy);
	UFUNCTION()
	void OnTargetIsDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnDeathCameraTargetDied(AActor* DeadTarget);

	void TrySpectateImmidiatelyAfterDeath();

	AActor* LastDeathCameraTarget;

	FTimerHandle TimerHandle_SpectatePawn;
};
