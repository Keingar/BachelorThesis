// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugSpawnPoint.h"

ADebugSpawnPoint::ADebugSpawnPoint()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	RootComponent = MeshComp;
	SetActorHiddenInGame(true);
}

void ADebugSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}



