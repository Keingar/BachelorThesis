// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum EItemSubCategory : uint8 {
	LongSword UMETA(DisplayName = "LongSword"),
	Polearm UMETA(DisplayName = "Polearm"),
	LightHelmet UMETA(DisplayName = "LightHelmet"),
	LightLegs UMETA(DisplayName = "LightLegs"),
	LightChest UMETA(DisplayName = "LightChest"),
	Bombs UMETA(DisplayName = "Bombs"),
	Heart UMETA(DisplayName = "Heart"),
	Shield UMETA(DisplayName = "Shield")

};