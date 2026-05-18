// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "multiplayerTEST/GameplayInterfaces/InteractInterface.h"
#include "multiplayerTEST/GameplayInterfaces/Resettable.h"
#include "InteractableActorCore.generated.h"

class UObjectRegistryComponent;

UCLASS()
class MULTIPLAYERTEST_API AInteractableActorCore : public AActor, public IInteractInterface, public IResettable
{
	GENERATED_BODY()
	
public:	
	AInteractableActorCore();

	void Interact_Implementation(AActor* UserActor)override;
	void ResetObject_Implementation() override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool needReset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UObjectRegistryComponent* RegistryComponent;
};
