// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "multiplayerTEST/CustomStructs/DropChance.h"
#include "LootManagerComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTEST_API ULootManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULootManagerComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<FDropChance> DropChances;

	UFUNCTION()
	void OnOwnerDeath(AActor* Owner);
	
	UPROPERTY(EditAnywhere, Category = "Loot")
	bool bSaveOnReset;
};
