// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShipPassangerGhostCore.generated.h"

class AShip_Visual;
class AShip_Ghost;

UCLASS()
class MULTIPLAYERTEST_API AShipPassangerGhostCore : public ACharacter
{
	GENERATED_BODY()

public:
	AShipPassangerGhostCore(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ship")
	AShip_Visual* VisualShip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ship")
	AShip_Ghost* GhostShip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ship")
	AActor* RealActor;

	virtual void CopySettingsFromActor(ACharacter* RealCharacter);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

};
