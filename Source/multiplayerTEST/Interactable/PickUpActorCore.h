// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActorCore.h"
#include "multiplayerTEST/CustomStructs/DataTableStructs/ItemDataForTable.h"
#include "PickUpActorCore.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API APickUpActorCore : public AInteractableActorCore
{
	GENERATED_BODY()
	
public:
	APickUpActorCore();

	virtual void Interact_Implementation(AActor* UserActor) override;

	void SetLoot(TArray<FItemDataForTable*> Loot) { DroppedItemsID = Loot; }

	void ResetObject_Implementation()override;

	bool SaveOnReset;
protected:
	void BeginPlay()override;

	bool isLooted; // just in case check
	TArray<FItemDataForTable*> DroppedItemsID;
	UPROPERTY()
	FVector DefaultlootLocation;
};
