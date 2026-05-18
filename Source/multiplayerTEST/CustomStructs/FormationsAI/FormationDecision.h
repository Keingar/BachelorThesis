#pragma once

#include "CoreMinimal.h"
#include "FormationDecision.generated.h"

UENUM(BlueprintType)
enum class EFormationType : uint8
{
	None    UMETA(DisplayName = "None"),
	Line    UMETA(DisplayName = "Line"),
	Circle  UMETA(DisplayName = "Circle"),
	Wedge   UMETA(DisplayName = "Wedge")
};

UENUM(BlueprintType)
enum class EFormationMemberRole : uint8
{
	Leader     UMETA(DisplayName = "Leader"),
	Defender   UMETA(DisplayName = "Defender"),
	Aggressive UMETA(DisplayName = "Aggressive"),
	Support    UMETA(DisplayName = "Support"),
	Backline   UMETA(DisplayName = "Backline"),
	Flanker    UMETA(DisplayName = "Flanker")
};

UENUM(BlueprintType)
enum class EFormationTuningPreset : uint8
{
	Compact  UMETA(DisplayName = "Compact"),
	Balanced UMETA(DisplayName = "Balanced"),
	Spread   UMETA(DisplayName = "Spread")
};

USTRUCT(BlueprintType)
struct FFormationDecision
{
	GENERATED_BODY()

	// Which formation to apply
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFormationType FormationType = EFormationType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFormationTuningPreset TuningPreset = EFormationTuningPreset::Balanced;

	// --- Line parameters ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSetLineParams = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LineMinSpacing = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LineMaxLineLength = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LineRowOffset = 200.f;

	// --- Circle parameters ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSetCircleParams = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CircleRadius = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CircleArcAngleDegrees = 360.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CircleArcOffsetDegrees = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CircleMinAnglePerMemberDeg = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CircleLayerRadiusOffset = 200.f;

	// --- Wedge parameters ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSetWedgeParams = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WedgeDesiredDistanceToTarget = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WedgeSideSpacing = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WedgeDepthSpacing = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WedgeLeaderOffset = 300.f;

	// Enable swapping low-health front-line members to backline when beneficial
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableBacklineSwap = false;

	// Threshold (percent) difference in normalized HP required to perform a swap (0..100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BacklineHpDiffThresholdPercent = 40.f;
};
