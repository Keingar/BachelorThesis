// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerTeestHUD.h"
#include "multiplayerTEST/UI/Health/BossHealthBar.h"
#include "Kismet/GameplayStatics.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTestGameStateBase.h"
#include "multiplayerTEST/UI/FullUIHUD.h"


void AMultiplayerTeestHUD::RemoveBossHealthBar(class AEnemyCore* CurrentBoss, bool isBossDefeated)
{
	if (FullUIHUD && FullUIHUD->BossHealthBar) {
		FullUIHUD->BossHealthBar->RemoveFromParent();
	}
}

void AMultiplayerTeestHUD::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwningPawn()) return;

	if (AMultiplayerTestGameStateBase* CustomGameState = Cast<AMultiplayerTestGameStateBase>(UGameplayStatics::GetGameState(GetWorld()))) {
		CustomGameState->BossFightStarted.AddDynamic(this, &AMultiplayerTeestHUD::DrawBossHealthBar);

		CustomGameState->BossFightEnded.AddDynamic(this, &AMultiplayerTeestHUD::RemoveBossHealthBar);
	}

}
