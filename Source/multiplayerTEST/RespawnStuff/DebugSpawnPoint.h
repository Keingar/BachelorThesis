// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugSpawnPoint.generated.h"

class UStaticMeshComponent;

UCLASS()
class MULTIPLAYERTEST_API ADebugSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ADebugSpawnPoint();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* MeshComp;
};
