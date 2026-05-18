// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EquipSystemInterface.generated.h"

class UWearableItem;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEquipSystemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MULTIPLAYERTEST_API IEquipSystemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual USkeletalMeshComponent* GetHelmetMesh() const = 0;
	virtual USkeletalMeshComponent* GetChestMesh() const = 0;
	virtual USkeletalMeshComponent* GetLegsMesh() const = 0;
	virtual USkeletalMeshComponent* GetWeaponRH1_Mesh() const = 0;
	virtual USkeletalMeshComponent* GetWeaponLH1_Mesh() const = 0;
	virtual USkeletalMeshComponent* GetHeartMesh() const = 0;

	virtual void EquipHelper(UWearableItem* NewItem, 
		USkeletalMeshComponent* EquipSlot,
		FName MaskParameterName,
		bool InvertRelativePosition
	) = 0;
};
