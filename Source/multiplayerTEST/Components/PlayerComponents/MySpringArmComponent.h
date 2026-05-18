// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "MySpringArmComponent.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UMySpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

protected:
	virtual void UpdateDesiredArmLocation(bool bDoTrace,
		bool bDoLocationLag,
		bool bDoRotationLag,
		float DeltaTime
	);

public:
	UPROPERTY(EditDefaultsonly)
	float MaxDownDistance = 0;
};
