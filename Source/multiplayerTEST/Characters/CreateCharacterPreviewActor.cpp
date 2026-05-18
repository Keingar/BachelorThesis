#include "CreateCharacterPreviewActor.h"
#include "multiplayerTEST/Items/WearableItem.h"
#include "multiplayerTEST/Items/Weapon.h"
#include "multiplayerTEST/CustomEnums/ItemSubCategory.h"

ACreateCharacterPreviewActor::ACreateCharacterPreviewActor()
{
	MainMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MainMesh"));

	HelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelmetMesh"));
	HelmetMesh->SetupAttachment(MainMesh);

	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestMesh"));
	ChestMesh->SetupAttachment(MainMesh);

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh"));
	LegsMesh->SetupAttachment(MainMesh);

	WeaponRHMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshRH"));
	WeaponRHMesh->SetupAttachment(MainMesh, "hand_r");

	WeaponLHMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshLH"));
	WeaponLHMesh->SetupAttachment(MainMesh, "hand_l");

	HeartMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeartMesh"));
	HeartMesh->SetupAttachment(MainMesh, "spine_05");

}

void ACreateCharacterPreviewActor::BeginPlay()
{
	Super::BeginPlay();

	if (!MainMesh) return; 
	
	UMaterialInterface* BaseMaterial = MainMesh->GetMaterial(0);
	if (!BaseMaterial) return;
	
	PlayerDynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (!PlayerDynamicMaterial) return;
	
	MainMesh->SetMaterial(0, PlayerDynamicMaterial);

	if (DefaultItemMask)
	{
		PlayerDynamicMaterial->SetTextureParameterValue(TEXT("HelmetMask"), DefaultItemMask);
		PlayerDynamicMaterial->SetTextureParameterValue(TEXT("ChestMask"), DefaultItemMask);
		PlayerDynamicMaterial->SetTextureParameterValue(TEXT("LegsMask"), DefaultItemMask);
	}
}

void ACreateCharacterPreviewActor::EquipItem(UItem* ItemToEquip)
{
	if (!ItemToEquip)
	{
		return;
	}

	UWearableItem* WearableItem = Cast<UWearableItem>(ItemToEquip);
	if (!WearableItem)
	{
		return;
	}

	switch (WearableItem->ItemSubCategory)
	{
	case EItemSubCategory::LightHelmet:
		EquipItemVisually(WearableItem, HelmetMesh, false);
		EquippedHelmet = WearableItem;
		UpdateMaskHelperFunction(WearableItem, TEXT("HelmetMask"));
		break;

	case EItemSubCategory::LightChest:
		EquipItemVisually(WearableItem, ChestMesh, false);
		EquippedChest = WearableItem;
		UpdateMaskHelperFunction(WearableItem, TEXT("ChestMask"));
		break;

	case EItemSubCategory::LightLegs:
		EquipItemVisually(WearableItem, LegsMesh, false);
		EquippedLegs = WearableItem;
		UpdateMaskHelperFunction(WearableItem, TEXT("LegsMask"));
		break;

	case EItemSubCategory::Heart:
		EquipItemVisually(WearableItem, HeartMesh, false);
		break;

	case EItemSubCategory::Polearm:
	case EItemSubCategory::LongSword:
	case EItemSubCategory::Shield:
		if (WearableItem->EquippedSlot == 1) {
			EquipItemVisually(WearableItem, WeaponRHMesh, false);
		}
		else {
			EquipItemVisually(WearableItem, WeaponLHMesh, true);
		}

		break;

	default:
		break;
	}
}

void ACreateCharacterPreviewActor::EquipItemVisually(UWearableItem* NewItem,
	USkeletalMeshComponent* EquipSlot,
	bool bInvertRelativePosition)
{
	if (!EquipSlot)
	{
		return;
	}

	if (!NewItem)
	{
		EquipSlot->SetSkinnedAssetAndUpdate(nullptr, true);
		return;
	}

	EquipSlot->SetSkinnedAssetAndUpdate(NewItem->SkeletMesh, true);

	FVector FinalLocation = NewItem->RelativeLocationOffset;
	FRotator FinalRotation = NewItem->RelativeRotationOffset;
	FVector ActualScale = NewItem->ScaleWhenEquipped;

	if (!bInvertRelativePosition)
	{
		EquipSlot->SetRelativeTransform(FTransform(FinalRotation.Quaternion(), FinalLocation, ActualScale));

		return;
	}

	FinalLocation *= -1;
	FinalRotation.Roll += 180;

	UWeapon* NewWeaponItem = Cast<UWeapon>(NewItem);

	if (!NewWeaponItem) return;

	if (NewWeaponItem->invertYawWhenEquipLH) {
		FinalRotation.Yaw += 180;
	}
	EquipSlot->SetRelativeTransform(FTransform(FinalRotation.Quaternion(), FinalLocation, ActualScale));
}

void ACreateCharacterPreviewActor::UpdateMaskHelperFunction(UWearableItem* ItemToGetMaskFrom, FName MaskParameter) {
	if (!PlayerDynamicMaterial || MaskParameter == NAME_None) return;

	if (!ItemToGetMaskFrom || !ItemToGetMaskFrom->ItemMaskTexture) {
		PlayerDynamicMaterial->SetTextureParameterValue(MaskParameter, DefaultItemMask);

		return;
	}

	UTexture* MaskTex = ItemToGetMaskFrom->ItemMaskTexture;

	if (!MaskTex) {
		PlayerDynamicMaterial->SetTextureParameterValue(MaskParameter, DefaultItemMask);

		return;
	}

	PlayerDynamicMaterial->SetTextureParameterValue(MaskParameter, MaskTex);
}

void ACreateCharacterPreviewActor::ResetPreviewMeshes()
{
	if (HelmetMesh)
	{
		HelmetMesh->SetSkinnedAssetAndUpdate(nullptr, true);
	}

	if (ChestMesh)
	{
		ChestMesh->SetSkinnedAssetAndUpdate(nullptr, true);
	}

	if (LegsMesh)
	{
		LegsMesh->SetSkinnedAssetAndUpdate(nullptr, true);
	}

	if (HeartMesh)
	{
		HeartMesh->SetSkinnedAssetAndUpdate(nullptr, true);
	}

	if (WeaponRHMesh)
	{
		WeaponRHMesh->SetSkinnedAssetAndUpdate(nullptr, true);
	}

	if (WeaponLHMesh)
	{
		WeaponLHMesh->SetSkinnedAssetAndUpdate(nullptr, true);
	}

	EquippedHelmet = nullptr;
	EquippedChest = nullptr;
	EquippedLegs = nullptr;

	if (!PlayerDynamicMaterial && MainMesh)
	{
		if (UMaterialInterface* BaseMaterial = MainMesh->GetMaterial(0))
		{
			PlayerDynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (PlayerDynamicMaterial)
			{
				MainMesh->SetMaterial(0, PlayerDynamicMaterial);
			}
		}
	}

	if (PlayerDynamicMaterial)
	{
		PlayerDynamicMaterial->SetTextureParameterValue(TEXT("HelmetMask"), DefaultItemMask);
		PlayerDynamicMaterial->SetTextureParameterValue(TEXT("ChestMask"), DefaultItemMask);
		PlayerDynamicMaterial->SetTextureParameterValue(TEXT("LegsMask"), DefaultItemMask);
	}
}

void ACreateCharacterPreviewActor::SetMaterial(UMaterialInterface* NewMaterial) {
	if (!NewMaterial || !MainMesh)
		return;

	PlayerDynamicMaterial = UMaterialInstanceDynamic::Create(NewMaterial, this);
	if (!PlayerDynamicMaterial)
		return;

	MainMesh->SetMaterial(0, PlayerDynamicMaterial);

	UpdateAllMasks();
}

void ACreateCharacterPreviewActor::UpdateAllMasks()
{
	if (!PlayerDynamicMaterial)
	{
		if (MainMesh)
		{
			if (UMaterialInterface* BaseMaterial = MainMesh->GetMaterial(0))
			{
				PlayerDynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
				if (PlayerDynamicMaterial)
				{
					MainMesh->SetMaterial(0, PlayerDynamicMaterial);
				}
			}
		}

		if (!PlayerDynamicMaterial)
		{
			return;
		}
	}

	UpdateMaskHelperFunction(EquippedHelmet, TEXT("HelmetMask"));
	UpdateMaskHelperFunction(EquippedChest, TEXT("ChestMask"));
	UpdateMaskHelperFunction(EquippedLegs, TEXT("LegsMask"));
}