#pragma once

#include "CoreMinimal.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/FormationDecision.h"

class AEnemyCore;

struct FPBMADSlotCorrectionSettings
{
	bool bProjectCorrectedSlotsToNavMesh = true;
	bool bEnablePredictiveSlotAvoidance = true;
	bool bEnableSlotCorrectionCohesion = true;
	bool bEnableRoleAwareSlotCorrection = true;
	bool bDrawDebug = false;
	bool bDrawRadii = false;
	bool bDrawPredictionLinks = false;

	int32 Iterations = 3;
	float MinDistance = 140.f;
	float Strength = 0.5f;
	float MaxOffset = 220.f;
	float PredictionTime = 0.35f;
	float PreferredVelocityBlend = 0.65f;
	float RadiusScale = 1.f;
	float ShapePreservationStrength = 0.15f;
	float CohesionStrength = 0.08f;
};

struct FPBMADSlotCorrectionMember
{
	AEnemyCore* Member = nullptr;
	FVector CurrentLocation = FVector::ZeroVector;
	FVector CurrentVelocity = FVector::ZeroVector;
	FVector OriginalPosition = FVector::ZeroVector;
	FVector CorrectedPosition = FVector::ZeroVector;
	FVector PredictedPosition = FVector::ZeroVector;
	FVector DesiredDirection = FVector::ZeroVector;
	EFormationMemberRole Role = EFormationMemberRole::Defender;
	float Mobility = 1.f;
	float ShapePreservation = 0.7f;
	float MaxOffsetScale = 1.f;
	float CollisionRadius = 70.f;
	bool bIsLeader = false;
};

class MULTIPLAYERTEST_API FFormationSlotCorrectionSolver
{
public:
	static void CorrectSlots(
		UWorld* World,
		const FPBMADSlotCorrectionSettings& Settings,
		TArray<FPBMADSlotCorrectionMember>& Members
	);
};
