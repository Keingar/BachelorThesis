// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum GamePhase : uint8{
	OpenWorld UMETA(DisplayName = "OpenWorld"),
	BossFight UMETA(DisplayName = "BossFight"),
};