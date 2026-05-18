// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LockOnCameraParams.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FLockOnCameraParams
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOnCameraInfo", meta = (ClampMin = "0.0", ClampMax="5", ToolTip = "0 = camera dont change, 0.5 further by 50%"))
	float ExtraCameraLengthInPercent; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOnCameraInfo", meta = (ToolTip = "How much camera will go up or down when close to enemy"))
	float MaxExtraZ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOnCameraInfo", meta = (ToolTip = "How much camera will tilt down"))
	float MaxDownPitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOnCameraInfo", meta = (ClampMin = "0.0", ToolTip = "At what distance apply tilt fully"))
	float MinDistanceForFullPitch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOnCameraInfo", meta = (ClampMin = "0.0", ToolTip = "At what distance apply zoom out and extra Z fully"))
	float MinDistanceForZoomOut;
};
