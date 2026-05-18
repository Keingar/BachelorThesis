// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "multiplayerTEST/CustomStructs/SocketInfo.h"
#include "multiplayerTEST/CustomStructs/LockOnCameraParams.h"
#include "LockOnInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULockOnInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MULTIPLAYERTEST_API ILockOnInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "LockOn")
	bool CanBeLockedOn() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "LockOn")
	const TArray<FSocketInfo> GetLockOnSocketNames() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "LockOn")
	USkeletalMeshComponent* GetLockOnTargetMesh() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "LockOn")
	void OnLockedOnByActor(FName SelectedSocket);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "LockOn")
	void StopLockOn();

	virtual TArray<FLockOnCameraParams> GetLockOnCameraParams() = 0;
};
