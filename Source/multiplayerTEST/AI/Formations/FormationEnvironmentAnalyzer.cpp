#include "FormationEnvironmentAnalyzer.h"

#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"

namespace
{
	float ProbeNavigationDistance(
		UNavigationSystemV1* NavSystem,
		const FVector& Origin,
		const FVector& Direction,
		const FFormationEnvironmentAnalyzerSettings& Settings)
	{
		if (!NavSystem)
		{
			return 0.f;
		}

		const float Step = FMath::Max(25.f, Settings.ProbeStep);
		const float MaxDistance = FMath::Max(Step, Settings.MaxProbeDistance);
		float LastValidDistance = 0.f;

		for (float Distance = Step; Distance <= MaxDistance + KINDA_SMALL_NUMBER; Distance += Step)
		{
			const FVector ProbePoint = Origin + Direction * Distance;
			FNavLocation ProjectedLocation;
			if (!NavSystem->ProjectPointToNavigation(ProbePoint, ProjectedLocation, Settings.ProjectionExtent))
			{
				break;
			}

			FVector ProjectionDelta = ProjectedLocation.Location - ProbePoint;
			ProjectionDelta.Z = 0.f;
			if (ProjectionDelta.Size() > Settings.ProjectionTolerance)
			{
				break;
			}

			LastValidDistance = Distance;
		}

		return LastValidDistance;
	}

	const TCHAR* GetEnvironmentModeDebugName(EFormationEnvironmentMode Mode)
	{
		switch (Mode)
		{
		case EFormationEnvironmentMode::Open:
			return TEXT("Open");
		case EFormationEnvironmentMode::Limited:
			return TEXT("Limited");
		case EFormationEnvironmentMode::NarrowCorridor:
			return TEXT("NarrowCorridor");
		case EFormationEnvironmentMode::BlockedOrInvalid:
			return TEXT("BlockedOrInvalid");
		default:
			return TEXT("Unknown");
		}
	}
}

FFormationEnvironmentAnalysisResult FFormationEnvironmentAnalyzer::Analyze(
	UWorld* World,
	const FFormationEnvironmentAnalyzerSettings& Settings,
	const FFormationEnvironmentAnalysisInput& Input)
{
	FFormationEnvironmentAnalysisResult Result;
	Result.Origin = Input.Origin;

	if (!World)
	{
		return Result;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
	if (!NavSystem)
	{
		return Result;
	}

	FNavLocation ProjectedOrigin;
	if (!NavSystem->ProjectPointToNavigation(Input.Origin, ProjectedOrigin, Settings.ProjectionExtent))
	{
		Result.Mode = EFormationEnvironmentMode::BlockedOrInvalid;
		if (Settings.bDrawDebug)
		{
			DrawDebugSphere(World, Input.Origin, 40.f, 12, FColor::Red, false, 1.f);
		}
		return Result;
	}

	Result.Origin = ProjectedOrigin.Location;

	FVector Forward = FVector::VectorPlaneProject(Input.Forward, FVector::UpVector).GetSafeNormal();
	if (Forward.IsNearlyZero())
	{
		Forward = FVector::ForwardVector;
	}

	const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
	Result.Forward = Forward;
	Result.Right = Right.IsNearlyZero() ? FVector::RightVector : Right;

	Result.AvailableLeft = ProbeNavigationDistance(NavSystem, Result.Origin, -Result.Right, Settings);
	Result.AvailableRight = ProbeNavigationDistance(NavSystem, Result.Origin, Result.Right, Settings);
	Result.AvailableForward = ProbeNavigationDistance(NavSystem, Result.Origin, Result.Forward, Settings);
	Result.AvailableBackward = ProbeNavigationDistance(NavSystem, Result.Origin, -Result.Forward, Settings);
	Result.AvailableWidth = Result.AvailableLeft + Result.AvailableRight;
	Result.AvailableDepth = Result.AvailableForward + Result.AvailableBackward;
	Result.CorridorDirection = Result.AvailableDepth >= Result.AvailableWidth ? Result.Forward : Result.Right;

	const float DesiredWidth = FMath::Max(Input.DesiredFormationWidth, 1.f);
	Result.WidthScale = FMath::Clamp(Result.AvailableWidth / DesiredWidth, 0.f, 1.f);
	Result.bValid = true;

	const float MaxProbeDistance = FMath::Max(FMath::Max(25.f, Settings.ProbeStep), Settings.MaxProbeDistance);
	const bool bReachedProbeLimitOnBothSides =
		Result.AvailableLeft >= MaxProbeDistance - Settings.ProbeStep * 0.5f &&
		Result.AvailableRight >= MaxProbeDistance - Settings.ProbeStep * 0.5f;

	if (bReachedProbeLimitOnBothSides)
	{
		Result.WidthScale = 1.f;
		Result.Mode = EFormationEnvironmentMode::Open;
	}
	else if (Result.WidthScale <= Settings.NarrowWidthScaleThreshold)
	{
		Result.Mode = EFormationEnvironmentMode::NarrowCorridor;
	}
	else if (Result.WidthScale <= Settings.LimitedWidthScaleThreshold)
	{
		Result.Mode = EFormationEnvironmentMode::Limited;
	}
	else
	{
		Result.Mode = EFormationEnvironmentMode::Open;
	}

	if (Settings.bDrawDebug)
	{
		const FColor ModeColor = Result.Mode == EFormationEnvironmentMode::Open
									 ? FColor::Green
									 : (Result.Mode == EFormationEnvironmentMode::Limited ? FColor::Yellow : FColor::Red);
		const FVector UpOffset(0.f, 0.f, 35.f);

		DrawDebugSphere(World, Result.Origin + UpOffset, 32.f, 12, ModeColor, false, 1.f);
		DrawDebugLine(World, Result.Origin + UpOffset, Result.Origin - Result.Right * Result.AvailableLeft + UpOffset, FColor::Cyan, false, 1.f, 0, 3.f);
		DrawDebugLine(World, Result.Origin + UpOffset, Result.Origin + Result.Right * Result.AvailableRight + UpOffset, FColor::Cyan, false, 1.f, 0, 3.f);
		DrawDebugLine(World, Result.Origin + UpOffset, Result.Origin + Result.Forward * Result.AvailableForward + UpOffset, FColor::Blue, false, 1.f, 0, 2.f);
		DrawDebugLine(World, Result.Origin + UpOffset, Result.Origin - Result.Forward * Result.AvailableBackward + UpOffset, FColor::Blue, false, 1.f, 0, 2.f);
		DrawDebugDirectionalArrow(World, Result.Origin + UpOffset, Result.Origin + Result.CorridorDirection * 250.f + UpOffset, 80.f, ModeColor, false, 1.f, 0, 4.f);
		DrawDebugString(
			World,
			Result.Origin + FVector(0.f, 0.f, 120.f),
			FString::Printf(TEXT("%s %s Width=%.0f Scale=%.2f"), *Settings.DebugLabel, GetEnvironmentModeDebugName(Result.Mode), Result.AvailableWidth, Result.WidthScale),
			nullptr,
			ModeColor,
			1.f,
			false);
	}

	return Result;
}

FFormationEnvironmentAnalysisResult FFormationEnvironmentAnalyzer::SelectMostRestrictive(
	const FFormationEnvironmentAnalysisResult& A,
	const FFormationEnvironmentAnalysisResult& B)
{
	if (!A.bValid)
	{
		return B;
	}

	if (!B.bValid)
	{
		return A;
	}

	if (B.WidthScale < A.WidthScale)
	{
		return B;
	}

	if (FMath::IsNearlyEqual(B.WidthScale, A.WidthScale) && B.AvailableWidth < A.AvailableWidth)
	{
		return B;
	}

	return A;
}
