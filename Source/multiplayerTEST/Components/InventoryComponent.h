// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "multiplayerTEST/CustomEnums/ItemSubCategory.h"
#include "InventoryComponent.generated.h"

class UItem;
class UWearableItem;
class IEquipSystemInterface;
class UItemSubsystem;
class USaveGameSubsystem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, TEnumAsByte<EItemSubCategory>);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTEST_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool AddItem(UItem* Item);
	bool RemoveItem(UItem* Item);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<UItem*> MiscItems;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<UItem*> HelmetItems;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<UItem*> ChestItems;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<UItem*> LegsItems;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<UItem*> WeaponItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UWearableItem* SaveeHelmet;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UWearableItem* SaveChest;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UWearableItem* SaveLegs;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UWearableItem* SaveWeaponRH1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UWearableItem* SaveWeaponLH1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UWearableItem* SaveHeartItem;

	UPROPERTY()
	APawn* AvatarActor;
	IEquipSystemInterface* AvatarEquipInterface;

	FOnInventoryUpdated OnUpdatedInventory;
public:
	void EquipHelmet(UWearableItem* NewHelmet);
	void EquipChest(UWearableItem* NewChest); 
	void EquipLegs(UWearableItem* NewLegs);
	void EquipWeaponRH1(UWearableItem* NewWeapon);
	void EquipWeaponLH1(UWearableItem* NewWeapon);
	void EquipHeart(UWearableItem* NewHeart);

	void TakeOffHelmet();
	void TakeOffChest();
	void TakeOffLegs();
	void TakeOffWeaponRH1();
	void TakeOffWeaponLH1();
	void TakeOffHeart();

	bool GetbIsInventoryInitolized() { return bIsInventoryInitolized;}

	TArray<UItem*>* GetItemArrayFromItemSubCategory(TEnumAsByte<EItemSubCategory> ItemCategory);

	void AfterDeathSetUp(APawn* NewAvatar);

protected:

	void EquipFunctionHelper(UWearableItem* NewItem, UWearableItem** CorrespondingSavedItem);
	void TakeOffFunctionHelper(UWearableItem** SavedItem);

	UFUNCTION(Server, Reliable)
	void VisualHelmetEquip_Server(int NewHelmetID);

	UFUNCTION(Server, Reliable)
	void VisualChestEquip_Server(int newChestID);

	UFUNCTION(Server, Reliable)
	void VisualLegsEquip_Server(int newLegsID);

	UFUNCTION(Server, Reliable)
	void VisualHeartEquip_Server(int NewHeartID);

	UFUNCTION(Server, Reliable)
	void VisualWeaponEquipRH1_Server(int newWeaponID);

	UFUNCTION(Server, Reliable)
	void VisualWeaponEquipLH1_Server(int newWeaponID);

	UPROPERTY()
	UItemSubsystem* ItemSubsystem;
	UPROPERTY()
	USaveGameSubsystem* SaveGameSubsystem;

	void SetUpInventoryComponentData();

	void SetUpSavedItems();

// for replication

protected:
	UFUNCTION()
	void OnRep_Helmet();
	UFUNCTION()
	void OnRep_Chest();
	UFUNCTION()
	void OnRep_Legs();
	UFUNCTION()
	void OnRep_WeaponRH1();
	UFUNCTION()
	void OnRep_WeaponLH1();
	UFUNCTION()
	void OnRep_Heart();
	// also the slot from character and mask parameter name
	void HandleEquipRep(int32 ItemID,
		UWearableItem*& SlotToSaveNewItemIn,
		USkeletalMeshComponent* SlotEquipTo,
		FName MaskParameterName = NAME_None,
		bool InvertRelativePosition = false
	);

	UPROPERTY(ReplicatedUsing = OnRep_Helmet)
	int SaveHelmetID;
	UPROPERTY(ReplicatedUsing = OnRep_Chest)
	int SaveChestID;
	UPROPERTY(ReplicatedUsing = OnRep_Legs)
	int SaveLegsID;
	UPROPERTY(ReplicatedUsing = OnRep_WeaponRH1)
	int SaveWeaponRH1_ID;
	UPROPERTY(ReplicatedUsing = OnRep_WeaponLH1)
	int SaveWeaponLH1_ID;
	UPROPERTY(ReplicatedUsing = OnRep_Heart)
	int SaveHeartID;

	void SetHelmetItemID(int newHelmetID);
	void SetChestItemID(int newChestID);
	void SetLegsItemID(int newLegsID);
	void SetWeaponRH1_ID(int newWeaponID);
	void SetWeaponLH1_ID(int newWeaponID);
	void SetHeartItemID(int newHeartID);

	void SortItemsById(TArray<UItem*>* ItemsArray);

	bool bIsInventoryInitolized = false;
};
