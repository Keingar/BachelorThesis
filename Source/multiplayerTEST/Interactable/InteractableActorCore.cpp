// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableActorCore.h"
#include "multiplayerTEST/Components/OtherComponents/ObjectRegistryComponent.h"

AInteractableActorCore::AInteractableActorCore()
{
	RegistryComponent = CreateDefaultSubobject<UObjectRegistryComponent>("RegistryComp");

	needReset = true;
}

void AInteractableActorCore::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractableActorCore::Interact_Implementation(AActor* UserActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Inside Interct Actor %s Interact function is not set up"), *GetOwner()->GetName());
}

void AInteractableActorCore::ResetObject_Implementation()
{
	UE_LOG(LogTemp, Error, TEXT("Not overriden reset function for %s"), *GetName())
}