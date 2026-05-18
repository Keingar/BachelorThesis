// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GetOwningMainMesh.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UGetOwningMainMesh : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MULTIPLAYERTEST_API IGetOwningMainMesh
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Mesh")
	UMeshComponent* GetOwningMainMesh() const;
};
