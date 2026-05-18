// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "multiplayerTEST/CustomEnums/ItemSubCategory.h"
#include "Item.generated.h"

class USkeletalMeshComponent;
class UTexture2D;
class UInventoryComponent;

UCLASS(Abstract, BlueprintType, Blueprintable, DefaultToInstanced)
class MULTIPLAYERTEST_API UItem : public UObject
{
	GENERATED_BODY()
	
public:

	UItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	bool IsStackable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	int MaxStack;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int Quantity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	int Id;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText UseActionText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	USkeletalMesh* SkeletMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	UTexture2D* Thumbnail;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText ItemDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (MultiLine = true))
	FText ItemDescription;

	UPROPERTY()
	UInventoryComponent* OwningInventory;

	virtual void Use() { check(0 && "You must override this");}

	UFUNCTION(BlueprintImplementableEvent)
	void OnUse();

	bool CanBeUsed();

	UFUNCTION(BlueprintCallable)
	bool UseItemFromInterface();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TEnumAsByte<EItemSubCategory> ItemSubCategory;
};
