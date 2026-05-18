// Fill out your copyright notice in the Description page of Project Settings.


#include "FogWall.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "multiplayerTEST/GameplayInterfaces/EnvironmentInteractInterface.h"

AFogWall::AFogWall()
{
    AttachedBoss = nullptr;
    WhereToTeleport = nullptr;
}

void AFogWall::BeginPlay()
{
	Super::BeginPlay();
    if (UHealthComponent* BossHealthComponent = AttachedBoss->GetComponentByClass<UHealthComponent>()) {
        BossHealthComponent->ActorDied.AddDynamic(this, &AFogWall::DestroyFogWall);
    }
}

void AFogWall::DestroyFogWall(AActor* DiedActor)
{
    Destroy();
}

void AFogWall::Interact_Implementation(class AActor* UserActor)
{
  //  IBossInterface* ActorAsBoss = Cast<IBossInterface>(AttachedBoss);

  //  if(!ActorAsBoss) {
      //  return;
  //  }
    
   // if (!ActorAsBoss->StartFight(UserActor)) {
 //       return;
 //   }

    IEnvironmentInteractInterface* TeleportableActor = Cast<IEnvironmentInteractInterface>(UserActor);
    
    if (TeleportableActor) {
        TeleportableActor->TeleportActor(WhereToTeleport->GetActorLocation(), WhereToTeleport->GetActorRotation());
    }
}
