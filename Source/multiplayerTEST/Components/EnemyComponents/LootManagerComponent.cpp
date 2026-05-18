// Fill out your copyright notice in the Description page of Project Settings.


#include "LootManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "GameFramework/GameModeBase.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/ItemSubsystem.h"

ULootManagerComponent::ULootManagerComponent()
{

}

void ULootManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ROLE_Authority) return; 

	if(UHealthComponent* HealthComponent = GetOwner()->FindComponentByClass<UHealthComponent>())
	{
		HealthComponent->ActorDied.AddDynamic(this, &ULootManagerComponent::OnOwnerDeath);
	}

}

void ULootManagerComponent::OnOwnerDeath(AActor* Owner)
{
	if (!Owner) {
		return;
	}

	TArray<int> Loot;

	for (FDropChance& Drop : DropChances)
	{
		if (FMath::FRandRange(0.f, 1.f) <= Drop.DropChance)
		{
			Loot.Add(Drop.ItemID);
		}
	}
	
	if(Loot.IsEmpty()) return;
	
	UItemSubsystem* ItemSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>();
	if (!ItemSubsystem) return;

	USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh) return;

	FVector DropLocation = Mesh->GetComponentLocation();

	// Call function in GameMode to spawn pickups
	ItemSubsystem->SpawnDroppedItemsAtLocation(Loot, DropLocation, bSaveOnReset);
}
