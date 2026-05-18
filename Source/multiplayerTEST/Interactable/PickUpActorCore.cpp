// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpActorCore.h"
#include "multiplayerTEST/Components/InventoryComponent.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/ItemSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "multiplayerTEST/Components/OtherComponents/ObjectRegistryComponent.h"

APickUpActorCore::APickUpActorCore()
{
	isLooted = false;
}

void APickUpActorCore::BeginPlay()
{
	Super::BeginPlay();

	RegistryComponent->ObjectID = FName(*FGuid::NewGuid().ToString());

	DefaultlootLocation = GetActorLocation();
}


void APickUpActorCore::Interact_Implementation(AActor* UserActor)
{
	if (!HasAuthority() || !UserActor || isLooted) return;
	
	AController* Controller = Cast<AController>(UserActor->GetOwner());
	if (!Controller) return;

	APlayerState* PlayerState = Controller->GetPlayerState<APlayerState>();
	if (!PlayerState) return;

	UInventoryComponent* InventoryComponent = PlayerState->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent) return;

	UItemSubsystem* ItemSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>();
	if (!ItemSubsystem) return;

	isLooted = true;

	for (FItemDataForTable* ItemData : DroppedItemsID)
	{
		UItem* NewItem = ItemSubsystem->GetItemInstanceFromItemData(ItemData, InventoryComponent);

		if (!NewItem) continue;

		InventoryComponent->AddItem(NewItem);
	}

	Destroy();
}

void APickUpActorCore::ResetObject_Implementation()
{
	if (!SaveOnReset || isLooted) {
		Destroy();
		return;
	}
	if (GetActorLocation() == DefaultlootLocation) return; // with current code always true

	SetActorLocation(DefaultlootLocation, false);
}