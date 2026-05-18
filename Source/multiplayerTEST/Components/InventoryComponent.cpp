// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "multiplayerTEST/Items/WearableItem.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTestPlayerState.h"
#include "multiplayerTEST/SaveSystem/CharacterData.h"
#include "multiplayerTEST/GameplayInterfaces/EquipSystemInterface.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/ItemSubsystem.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/SaveGameSubsystem.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	bIsInventoryInitolized = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	SetUpInventoryComponentData();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, SaveHelmetID);
	DOREPLIFETIME(UInventoryComponent, SaveChestID);
	DOREPLIFETIME(UInventoryComponent, SaveLegsID);
	DOREPLIFETIME(UInventoryComponent, SaveWeaponRH1_ID);
	DOREPLIFETIME(UInventoryComponent, SaveWeaponLH1_ID);
	DOREPLIFETIME(UInventoryComponent, SaveHeartID);
}

void UInventoryComponent::SetUpInventoryComponentData() {

	FTimerHandle TimerHandle;

	AMultiplayerTestPlayerState* OwnerPlayerState = Cast<AMultiplayerTestPlayerState>(GetOwner());

	if (!OwnerPlayerState) {
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UInventoryComponent::SetUpInventoryComponentData, 0.1f, false);
		return;
	}

	AvatarActor = OwnerPlayerState->GetPawn();

	if (!AvatarActor) {
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UInventoryComponent::SetUpInventoryComponentData, 0.1f, false);
		return;
	}

	AvatarEquipInterface = Cast<IEquipSystemInterface>(AvatarActor);


	ItemSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>();
	SaveGameSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USaveGameSubsystem>();

	SetUpSavedItems();

	// only when local machine
	if (AvatarActor->IsLocallyControlled())
	{
		SaveGameSubsystem->SetLocalInventoryComponent(this);
	}

	bIsInventoryInitolized = true;
}

bool UInventoryComponent::AddItem(UItem* Item)
{
	if (!Item) return false;

	Item->OwningInventory = this;

	TArray<UItem*>* CategoryArray = GetItemArrayFromItemSubCategory(Item->ItemSubCategory);
	if (!CategoryArray) return false;

	if (Item->IsStackable)
	{
		for (UItem* CurrentItem : *CategoryArray)
		{
			if (CurrentItem->Id == Item->Id && CurrentItem->MaxStack > CurrentItem->Quantity)
			{
				int32 SpaceLeft = CurrentItem->MaxStack - CurrentItem->Quantity;
				int32 AmountToAdd = FMath::Min(SpaceLeft, Item->Quantity);

				CurrentItem->Quantity += AmountToAdd;
				Item->Quantity -= AmountToAdd;

				if (Item->Quantity == 0)
				{
					SortItemsById(CategoryArray);

					SaveGameSubsystem->MarkDirty();
					OnUpdatedInventory.Broadcast(Item->ItemSubCategory);

					return true;
				}
			}
		}
	}

	CategoryArray->Add(Item);

	SortItemsById(CategoryArray);

	SaveGameSubsystem->MarkDirty();
	OnUpdatedInventory.Broadcast(Item->ItemSubCategory);

	return true;
}


bool UInventoryComponent::RemoveItem(UItem* Item)
{
	if (Item && Item->Quantity == 0)
	{
		GetItemArrayFromItemSubCategory(Item->ItemSubCategory)->RemoveSingle(Item);

		Item->OwningInventory = nullptr;
		SaveGameSubsystem->MarkDirty();
		OnUpdatedInventory.Broadcast(Item->ItemSubCategory);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Failed to remove item from inventory in %s"), *this->GetName());

	return false;
}

void UInventoryComponent::EquipHelmet(class UWearableItem* NewHelmet)
{
	EquipFunctionHelper(NewHelmet, &SaveeHelmet);
	VisualHelmetEquip_Server(NewHelmet->Id);
	SaveeHelmet = NewHelmet;
	if (GetOwnerRole() != ROLE_Authority) SetHelmetItemID(NewHelmet->Id);
}

void UInventoryComponent::EquipChest(class UWearableItem* NewChest)
{
	EquipFunctionHelper(NewChest, &SaveChest);
	VisualChestEquip_Server(NewChest->Id);
	SaveChest = NewChest;
	if(GetOwnerRole() != ROLE_Authority) SetChestItemID(NewChest->Id);
}

void UInventoryComponent::EquipLegs(class UWearableItem* NewLegs)
{
	EquipFunctionHelper(NewLegs, &SaveLegs);
	VisualLegsEquip_Server(NewLegs->Id);

	SaveLegs = NewLegs;

	if (GetOwnerRole() != ROLE_Authority) SetLegsItemID(NewLegs->Id);
}

void UInventoryComponent::EquipWeaponRH1(class UWearableItem* NewWeapon)
{
	EquipFunctionHelper(NewWeapon, &SaveWeaponRH1);
	VisualWeaponEquipRH1_Server(NewWeapon->Id);
	SaveWeaponRH1 = NewWeapon;
	if (GetOwnerRole() != ROLE_Authority) SetWeaponRH1_ID(NewWeapon->Id);
}

void UInventoryComponent::EquipWeaponLH1(UWearableItem* NewWeapon)
{
	EquipFunctionHelper(NewWeapon, &SaveWeaponLH1);
	VisualWeaponEquipLH1_Server(NewWeapon->Id);
	SaveWeaponLH1 = NewWeapon;
	if (GetOwnerRole() != ROLE_Authority) SetWeaponLH1_ID(NewWeapon->Id);
}

void UInventoryComponent::EquipHeart(UWearableItem* NewHeart)
{
	EquipFunctionHelper(NewHeart, &SaveHeartItem);
	VisualHeartEquip_Server(NewHeart->Id);
	SaveHeartItem = NewHeart;
	if (GetOwnerRole() != ROLE_Authority) SetHeartItemID(NewHeart->Id);
}

// it's assumed that 0 is like nullptr
void UInventoryComponent::TakeOffHelmet()
{
	TakeOffFunctionHelper(&SaveeHelmet);
	VisualHelmetEquip_Server(0);
	if (GetOwnerRole() != ROLE_Authority) SetHelmetItemID(0);
}

void UInventoryComponent::TakeOffChest()
{
	TakeOffFunctionHelper(&SaveChest);
	VisualChestEquip_Server(0);
	if (GetOwnerRole() != ROLE_Authority) SetChestItemID(0);
}

void UInventoryComponent::TakeOffLegs()
{
	TakeOffFunctionHelper(&SaveLegs);
	VisualLegsEquip_Server(0);
	if (GetOwnerRole() != ROLE_Authority) SetLegsItemID(0);
}

void UInventoryComponent::TakeOffWeaponRH1()
{
	TakeOffFunctionHelper(&SaveWeaponRH1);
	VisualWeaponEquipRH1_Server(0);
	if (GetOwnerRole() != ROLE_Authority) SetWeaponRH1_ID(0);
}

void UInventoryComponent::TakeOffWeaponLH1()
{
	TakeOffFunctionHelper(&SaveWeaponLH1);
	VisualWeaponEquipLH1_Server(0);
	if (GetOwnerRole() != ROLE_Authority) SetWeaponLH1_ID(0);
}

void UInventoryComponent::TakeOffHeart()
{
	TakeOffFunctionHelper(&SaveHeartItem);
	VisualHeartEquip_Server(0);
	if (GetOwnerRole() != ROLE_Authority) SetHeartItemID(0);
}

TArray<class UItem*>* UInventoryComponent::GetItemArrayFromItemSubCategory(TEnumAsByte<EItemSubCategory> ItemCategory)
{
	switch (ItemCategory)
	{
	case EItemSubCategory::LightHelmet:
		return &HelmetItems;
	case EItemSubCategory::LightChest:
		return &ChestItems;
	case EItemSubCategory::LightLegs:
		return &LegsItems;
	case EItemSubCategory::Polearm:
	case EItemSubCategory::LongSword:
	case EItemSubCategory::Shield:
		return &WeaponItems;
	case EItemSubCategory::Bombs:
	case EItemSubCategory::Heart:
		return &MiscItems;
	}

	UE_LOG(LogTemp, Error, TEXT("Didn't find corresponding array inside GetCorrectItemArrayFromItemCategory"));
	return &MiscItems;
}

void UInventoryComponent::AfterDeathSetUp(APawn* NewAvatar)
{
	if (!NewAvatar) return;

	AvatarActor = NewAvatar;

	AvatarEquipInterface = Cast<IEquipSystemInterface>(AvatarActor);
}

void UInventoryComponent::EquipFunctionHelper(UWearableItem* NewItem, UWearableItem** CorrespondingSavedItem)
{
	NewItem->bIsEquipped = true;

	if (*CorrespondingSavedItem != nullptr) {
		TakeOffFunctionHelper(CorrespondingSavedItem);
	}

	*CorrespondingSavedItem = NewItem;
	SaveGameSubsystem->MarkDirty();
	OnUpdatedInventory.Broadcast(NewItem->ItemSubCategory);
}

void UInventoryComponent::TakeOffFunctionHelper(UWearableItem** SavedItem)
{
	if (!SavedItem || !(*SavedItem) || !(*SavedItem)->bIsEquipped) return;
	
	(*SavedItem)->bIsEquipped = false;

	SaveGameSubsystem->MarkDirty();
	OnUpdatedInventory.Broadcast((*SavedItem)->ItemSubCategory);

	*SavedItem = nullptr;
}

void UInventoryComponent::VisualHelmetEquip_Server_Implementation(int NewHelmetID)
{
	SetHelmetItemID(NewHelmetID);
}

void UInventoryComponent::VisualChestEquip_Server_Implementation(int NewChestID)
{
	SetChestItemID(NewChestID);
}

void UInventoryComponent::VisualLegsEquip_Server_Implementation(int NewLegsID)
{
	SetLegsItemID(NewLegsID);
}

void UInventoryComponent::VisualWeaponEquipRH1_Server_Implementation(int NewWeaponID)
{
	SetWeaponRH1_ID(NewWeaponID);
}

void UInventoryComponent::VisualWeaponEquipLH1_Server_Implementation(int NewWeaponID)
{
	SetWeaponLH1_ID(NewWeaponID);
}

void UInventoryComponent::VisualHeartEquip_Server_Implementation(int NewHeartID)
{
	SetHeartItemID(NewHeartID);
}

void UInventoryComponent::SetUpSavedItems()
{
	if (!ItemSubsystem || !SaveGameSubsystem) {
		UE_LOG(LogTemp, Error, TEXT("Failed to set up saved items because one of subsystems is nullptr"));
		return;
	}

	UCharacterData* ChosenCharacter = SaveGameSubsystem->GetChosenCharacter();

	if (!ChosenCharacter) {
		UE_LOG(LogTemp, Error, TEXT("Failed to set up saved items because CharacterData is nullptr"));
		return;
	}

	TArray<FItemDataSave> SavedInventory = ChosenCharacter->GetCharacterData().SavedInventory;

	for (FItemDataSave& CurrentSavedItem : SavedInventory) {
		UItem* CurrentItemInstance = ItemSubsystem->GetItemInstanceFromID(CurrentSavedItem.ItemID, this);
	
		if (!CurrentItemInstance) continue;

		CurrentItemInstance->Quantity = CurrentSavedItem.Quantity;

		AddItem(CurrentItemInstance);

		if (!CurrentSavedItem.bIsEquipped)continue;
		UWearableItem* WearableItemInstance = Cast<UWearableItem>(CurrentItemInstance);

		if (!WearableItemInstance) {
			UE_LOG(LogTemp, Error, TEXT("Item saved as equipped but its not wearable %s"), *CurrentItemInstance->GetName());
		}

		switch (CurrentItemInstance->ItemSubCategory)
		{
		case EItemSubCategory::LightHelmet:
			EquipHelmet(WearableItemInstance);
			break;
		case EItemSubCategory::LightChest:
			EquipChest(WearableItemInstance);
			break;
		case EItemSubCategory::LightLegs:
			EquipLegs(WearableItemInstance);
			break;
		case EItemSubCategory::Polearm:
		case EItemSubCategory::LongSword:
		case EItemSubCategory::Shield:
			if (CurrentSavedItem.EquippedSlot == 1) {
				EquipWeaponRH1(WearableItemInstance);
				break;
			}

			EquipWeaponLH1(WearableItemInstance);
			break;
		case EItemSubCategory::Heart:
			EquipHeart(WearableItemInstance);
			break;
		}
	}
}

void UInventoryComponent::OnRep_Helmet()
{
	if (!AvatarEquipInterface) return;

	HandleEquipRep(SaveHelmetID, 
		SaveeHelmet,
		AvatarEquipInterface->GetHelmetMesh(),
		TEXT("HelmetMask")
	);
}

void UInventoryComponent::OnRep_Chest()
{
	if (!AvatarEquipInterface) return;

	HandleEquipRep(SaveChestID,
		SaveChest,
		AvatarEquipInterface->GetChestMesh(),
		TEXT("ChestMask")
	);
}

void UInventoryComponent::OnRep_Legs()
{
	if (!AvatarEquipInterface) return;

	HandleEquipRep(SaveLegsID,
		SaveLegs,
		AvatarEquipInterface->GetLegsMesh(),
		TEXT("LegsMask")
	);
}

void UInventoryComponent::OnRep_WeaponRH1()
{
	if (!AvatarEquipInterface) return;

	HandleEquipRep(SaveWeaponRH1_ID,
		SaveWeaponRH1,
		AvatarEquipInterface->GetWeaponRH1_Mesh()
	);
}

void UInventoryComponent::OnRep_WeaponLH1()
{
	if (!AvatarEquipInterface) return;

	HandleEquipRep(SaveWeaponLH1_ID,
		SaveWeaponLH1,
		AvatarEquipInterface->GetWeaponLH1_Mesh(),
		NAME_None,
		true
	);
}

void UInventoryComponent::OnRep_Heart()
{
	if (!AvatarEquipInterface) return;

	HandleEquipRep(SaveHeartID,
		SaveHeartItem,
		AvatarEquipInterface->GetHeartMesh()
	);
}

void UInventoryComponent::HandleEquipRep(int32 ItemID, 
	UWearableItem*& SlotToSaveNewItemIn, 
	USkeletalMeshComponent* SlotEquipTo, 
	FName MaskParameterName,
	bool InvertRelativePosition
	)
{
	if (!AvatarEquipInterface) { return; }

	UWearableItem* NewItem = Cast<UWearableItem>(ItemSubsystem->GetItemInstanceFromID(ItemID, this));
	if (NewItem) NewItem->OwningInventory = this;

	if (AvatarActor->GetLocalRole() == ROLE_Authority && AvatarActor && !AvatarActor->IsLocallyControlled()) {
		SlotToSaveNewItemIn = NewItem;
	}

	AvatarEquipInterface->EquipHelper(NewItem, SlotEquipTo, MaskParameterName, InvertRelativePosition);
}

void UInventoryComponent::SetHelmetItemID(int newHelmetID)
{
	SaveHelmetID = newHelmetID;
	OnRep_Helmet();
	AvatarActor->ForceNetUpdate();
}

void UInventoryComponent::SetChestItemID(int newChestID)
{
	SaveChestID = newChestID;
	OnRep_Chest();
	AvatarActor->ForceNetUpdate();
}

void UInventoryComponent::SetLegsItemID(int newLegsID)
{
	SaveLegsID = newLegsID;
	OnRep_Legs();
	AvatarActor->ForceNetUpdate();
}

void UInventoryComponent::SetWeaponRH1_ID(int NewWeaponID)
{
	SaveWeaponRH1_ID = NewWeaponID;
	OnRep_WeaponRH1();
	AvatarActor->ForceNetUpdate();
}

void UInventoryComponent::SetWeaponLH1_ID(int newWeaponID)
{
	SaveWeaponLH1_ID = newWeaponID;
	OnRep_WeaponLH1();
	AvatarActor->ForceNetUpdate();
}

void UInventoryComponent::SetHeartItemID(int newHeartID)
{
	SaveHeartID = newHeartID;
	OnRep_Heart();
	AvatarActor->ForceNetUpdate();
}

void UInventoryComponent::SortItemsById(TArray<UItem*>* ItemsArray)
{
	if (!ItemsArray) return;

	ItemsArray->Sort([](const UItem& A, const UItem& B)
		{
			return A.Id < B.Id;
		});
}
