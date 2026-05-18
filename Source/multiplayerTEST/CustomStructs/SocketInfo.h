// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SocketInfo.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FSocketInfo
{
	GENERATED_BODY()

public:
	FSocketInfo()
		: SocketName(TEXT(""))
		, bLockOnDownOrForward(false)
		, bLockOnUpOrBehind(false)
		, ChosenCameraCloseSettings(0)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SocketInfo", meta = (GetOptions = "GetSocketNames"))
	FName SocketName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SocketInfo", meta = (ToolTip = "False is forward true is down (Only for actors with more than 1 Lock on point)"))
	bool bLockOnDownOrForward;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SocketInfo", meta = (ToolTip = "False is behind true is up (Only for actors with more than 1 Lock on point)"))
	bool bLockOnUpOrBehind;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SocketInfo", meta = (ToolTip = "Will work only if in actor was set up camera params and then use here num that correspond to that array in actor"))
	int ChosenCameraCloseSettings;
	
};
