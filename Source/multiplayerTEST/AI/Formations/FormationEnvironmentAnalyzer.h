#pragma once

#include "CoreMinimal.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/FormationDecision.h"

enum class EFormationEnvironmentMode : uint8
{
	Open,
	Limited,
	NarrowCorridor,
	BlockedOrInvalid
};

struct FFormationEnvironmentAnalyzerSettings
{
	float ProbeStep = 100.f;
	float MaxProbeDistance = 900.f;
	FVector ProjectionExtent = FVector(50.f, 50.f, 250.f);
	float ProjectionTolerance = 45.f;
	float LimitedWidthScaleThreshold = 0.85f;
	float NarrowWidthScaleThreshold = 0.45f;
	FString DebugLabel = TEXT("Environment");
	bool bDrawDebug = false;
};

struct FFormationEnvironmentAnalysisInput
{
	FVector Origin = FVector::ZeroVector;
	FVector Forward = FVector::ForwardVector;
	float DesiredFormationWidth = 1.f;
	EFormationType FormationType = EFormationType::None;
};

struct FFormationEnvironmentAnalysisResult
{
	EFormationEnvironmentMode Mode = EFormationEnvironmentMode::Open;
	FVector Origin = FVector::ZeroVector;
	FVector Forward = FVector::ForwardVector;
	FVector Right = FVector::RightVector;
	FVector CorridorDirection = FVector::ForwardVector;
	float AvailableLeft = 0.f;
	float AvailableRight = 0.f;
	float AvailableForward = 0.f;
	float AvailableBackward = 0.f;
	float AvailableWidth = 0.f;
	float AvailableDepth = 0.f;
	float WidthScale = 1.f;
	bool bValid = false;
};

class MULTIPLAYERTEST_API FFormationEnvironmentAnalyzer
{
public:
	static FFormationEnvironmentAnalysisResult Analyze(
		UWorld* World,
		const FFormationEnvironmentAnalyzerSettings& Settings,
		const FFormationEnvironmentAnalysisInput& Input
	);

	static FFormationEnvironmentAnalysisResult SelectMostRestrictive(
		const FFormationEnvironmentAnalysisResult& A,
		const FFormationEnvironmentAnalysisResult& B
	);
};
