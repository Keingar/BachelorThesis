// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CreateCharacterPreviewActor.generated.h"


class USkeletalMeshComponent;
class UWearableItem;

UCLASS()
class MULTIPLAYERTEST_API ACreateCharacterPreviewActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* MainMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* HelmetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ChestMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* LegsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* HeartMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponRHMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponLHMesh;

public:	
	ACreateCharacterPreviewActor();

	UFUNCTION(BlueprintCallable)
	void EquipItem(class UItem* ItemToEquip);

	UFUNCTION(BlueprintCallable)
	void ResetPreviewMeshes();

	UFUNCTION(BlueprintCallable)
	void UpdateAllMasks();

protected:
	virtual void BeginPlay() override;
	
	void UpdateMaskHelperFunction(class UWearableItem* ItemToGetMaskFrom, FName MaskParameter = FName());

	void EquipItemVisually(class UWearableItem* NewItem,
		USkeletalMeshComponent* EquipSlot,
		bool bInvertRelativePosition
	);
	
	UPROPERTY()
	UMaterialInstanceDynamic* PlayerDynamicMaterial = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UTexture* DefaultItemMask;

	UFUNCTION(BlueprintCallable)
	void SetMaterial(UMaterialInterface* NewMaterial);

	UPROPERTY()
	UWearableItem* EquippedHelmet = nullptr;

	UPROPERTY()
	UWearableItem* EquippedChest = nullptr;

	UPROPERTY()
	UWearableItem* EquippedLegs = nullptr;
};
