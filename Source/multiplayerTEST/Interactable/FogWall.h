// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "multiplayerTEST/GameplayInterfaces/InteractInterface.h"
#include "FogWall.generated.h"

UCLASS()
class MULTIPLAYERTEST_API AFogWall : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	AFogWall();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	AActor* AttachedBoss;

	UPROPERTY(EditAnywhere)
	AActor* WhereToTeleport; // instead animation should be used

	UFUNCTION()
	void DestroyFogWall(AActor* DiedActor);

public:
	virtual void Interact_Implementation(AActor* UserActor) override;
};
