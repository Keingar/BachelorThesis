// Fill out your copyright notice in the Description page of Project Settings.


#include "BossHealthBar.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "HealthBase.h"
#include "multiplayerTEST/Characters/Enemies/EnemyCore.h"

void UBossHealthBar::NativeConstruct()
{
    Super::NativeConstruct();
}

void UBossHealthBar::SetUpHealthBar(UHealthComponent* OwnerHealthComp, FText CurrentBossName) {
    SetUpHealthDelegate(OwnerHealthComp);

    BossName->SetText(CurrentBossName);
}
