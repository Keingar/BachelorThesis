// Fill out your copyright notice in the Description page of Project Settings.

#include "GroupCommandComponent.h"
#include "multiplayerTEST/Characters/Enemies/EnemyCore.h"
#include "multiplayerTEST/Components/EnemyComponents/EnemyTargetingComponent.h"
#include "multiplayerTEST/Subsystems/WorldSubsystems/RegistrySubsystem.h"
#include "multiplayerTEST/RespawnStuff/EnemySpawner.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "multiplayerTEST/Controllers/CoreAIController.h"
#include "multiplayerTEST/AI/EnemyGroupTacticalAI.h"
#include "multiplayerTEST/AI/Formations/EnemyFormation.h"
#include "multiplayerTEST/AI/Formations/EnemyFormation_Circle.h"
#include "multiplayerTEST/AI/Formations/EnemyFormation_Line.h"
#include "multiplayerTEST/AI/Formations/EnemyFormation_Wedge.h"
#include "multiplayerTEST/AI/Formations/FormationEnvironmentAnalyzer.h"
#include "multiplayerTEST/AI/Formations/FormationSlotCorrectionSolver.h"
#include "DrawDebugHelpers.h"
#include "HAL/PlatformTime.h"
#include "NavigationSystem.h"
#include <limits>

namespace
{
	struct FFormationAttackCandidate
	{
		AEnemyCore *Member = nullptr;
		float DistSqToTarget = 0.f;
	};

	struct FGroupTargetSelection
	{
		AActor *TargetActor = nullptr;
		FVector TargetLocation = FVector::ZeroVector;
		bool bTargetIsEnemyGroup = false;
		int32 TargetGroupSize = 0;
		FVector TargetGroupCenter = FVector::ZeroVector;
	};

	bool IsAliveGroupMember(AEnemyCore *Member)
	{
		if (!IsValid(Member))
		{
			return false;
		}

		if (UHealthComponent *HealthComp = Member->GetHealthComponent())
		{
			return !HealthComp->GetIsDead();
		}

		return true;
	}

	int32 GetAliveGroupInfo(UGroupCommandComponent *Group, FVector &OutCenter)
	{
		OutCenter = FVector::ZeroVector;
		if (!Group)
		{
			return 0;
		}

		int32 AliveCount = 0;
		for (AEnemyCore *Member : Group->GroupMembers)
		{
			if (!IsAliveGroupMember(Member))
				continue;

			OutCenter += Member->GetActorLocation();
			AliveCount++;
		}

		if (AliveCount > 0)
		{
			OutCenter /= AliveCount;
		}

		return AliveCount;
	}

	FVector GetGroupReferenceLocation(UGroupCommandComponent *Group, UEnemyFormation *CurrentFormation, AEnemyCore *Leader)
	{
		if (CurrentFormation && CurrentFormation->HasValidAnchor())
		{
			return CurrentFormation->GetAnchorLocation();
		}

		if (IsValid(Leader))
		{
			return Leader->GetActorLocation();
		}

		return Group && Group->GetOwner() ? Group->GetOwner()->GetActorLocation() : FVector::ZeroVector;
	}

	FGroupTargetSelection SelectFormationTarget(UGroupCommandComponent *Group, UEnemyFormation *CurrentFormation, AEnemyCore *Leader, const TArray<AActor *> &KnownEnemies)
	{
		FGroupTargetSelection Selection;
		const FVector ReferenceLocation = GetGroupReferenceLocation(Group, CurrentFormation, Leader);
		float BestGroupDistSq = TNumericLimits<float>::Max();

		for (AActor *Candidate : KnownEnemies)
		{
			if (!IsValid(Candidate))
				continue;

			if (!Selection.TargetActor)
			{
				Selection.TargetActor = Candidate;
				Selection.TargetLocation = Candidate->GetActorLocation();
			}

			AEnemyCore *EnemyCandidate = Cast<AEnemyCore>(Candidate);
			if (!EnemyCandidate || !EnemyCandidate->OwningGroupCommandComponent || EnemyCandidate->OwningGroupCommandComponent == Group)
			{
				continue;
			}

			FVector CandidateCenter = FVector::ZeroVector;
			const int32 CandidateGroupSize = GetAliveGroupInfo(EnemyCandidate->OwningGroupCommandComponent, CandidateCenter);
			if (CandidateGroupSize < 3)
			{
				continue;
			}

			const float CandidateDistSq = FVector::DistSquared(ReferenceLocation, CandidateCenter);
			const bool bBetterGroup =
				!Selection.bTargetIsEnemyGroup ||
				CandidateGroupSize > Selection.TargetGroupSize ||
				(CandidateGroupSize == Selection.TargetGroupSize && CandidateDistSq < BestGroupDistSq);

			if (bBetterGroup)
			{
				Selection.TargetActor = Candidate;
				Selection.TargetLocation = CandidateCenter;
				Selection.bTargetIsEnemyGroup = true;
				Selection.TargetGroupSize = CandidateGroupSize;
				Selection.TargetGroupCenter = CandidateCenter;
				BestGroupDistSq = CandidateDistSq;
			}
		}

		return Selection;
	}

	EFormationMemberRole GetPreferredRoleForMember(AEnemyCore *Member, AEnemyCore *Leader)
	{
		if (Member && Member == Leader)
		{
			return EFormationMemberRole::Leader;
		}

		return Member ? Member->GetPreferredFormationRole() : EFormationMemberRole::Defender;
	}

	const TCHAR *GetFormationEnvironmentModeName(EFormationEnvironmentMode Mode)
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

	FVector GetActiveMembersCenter(const TArray<AEnemyCore *> &ActiveMembers)
	{
		FVector Center = FVector::ZeroVector;
		int32 Count = 0;
		for (AEnemyCore *Member : ActiveMembers)
		{
			if (!IsValid(Member))
				continue;

			Center += Member->GetActorLocation();
			Count++;
		}

		return Count > 0 ? Center / Count : FVector::ZeroVector;
	}

	float GetRoleSlotCost(
		EFormationMemberRole Role,
		const FVector &SlotLocation,
		const FVector &AnchorLocation,
		const FVector &Forward,
		const FVector &Right,
		float MinForward,
		float MaxForward,
		float MaxAbsLateral,
		float MaxRadius,
		bool bIsCircle)
	{
		const FVector Offset = SlotLocation - AnchorLocation;
		const float ForwardValue = FVector::DotProduct(Offset, Forward);
		const float AbsLateral = FMath::Abs(FVector::DotProduct(Offset, Right));
		const float Radius = Offset.Size2D();
		const float ForwardRange = FMath::Max(MaxForward - MinForward, 1.f);
		const float FrontCost = (MaxForward - ForwardValue) / ForwardRange;
		const float BackCost = (ForwardValue - MinForward) / ForwardRange;
		const float LateralAlpha = (MaxAbsLateral > KINDA_SMALL_NUMBER) ? FMath::Clamp(AbsLateral / MaxAbsLateral, 0.f, 1.f) : 0.f;
		const float OuterAlpha = (MaxRadius > KINDA_SMALL_NUMBER) ? FMath::Clamp(Radius / MaxRadius, 0.f, 1.f) : 0.f;
		const float CenterCost = LateralAlpha;
		const float EdgeCost = 1.f - LateralAlpha;
		const float OuterCost = 1.f - OuterAlpha;

		switch (Role)
		{
		case EFormationMemberRole::Leader:
			return 0.f;
		case EFormationMemberRole::Defender:
			return FrontCost * 300.f + CenterCost * 80.f;
		case EFormationMemberRole::Aggressive:
			return FrontCost * 340.f + CenterCost * 40.f;
		case EFormationMemberRole::Support:
			return (bIsCircle ? OuterCost * 220.f : FMath::Abs(BackCost - 0.35f) * 240.f) + CenterCost * 90.f;
		case EFormationMemberRole::Backline:
			return BackCost * 360.f + CenterCost * 70.f + (bIsCircle ? OuterCost * 120.f : 0.f);
		case EFormationMemberRole::Flanker:
			return EdgeCost * 360.f + FrontCost * 110.f;
		default:
			return 0.f;
		}
	}

	void ApplyTestingTuningPreset(FFormationDecision &Decision)
	{
		if (Decision.TuningPreset == EFormationTuningPreset::Balanced)
		{
			return;
		}

		const bool bCompact = Decision.TuningPreset == EFormationTuningPreset::Compact;
		const float TightFactor = bCompact ? 0.82f : 1.20f;
		const float DepthFactor = bCompact ? 0.80f : 1.25f;
		const float DistanceFactor = bCompact ? 0.90f : 1.10f;

		if (Decision.FormationType == EFormationType::Circle)
		{
			Decision.CircleRadius = FMath::Clamp(Decision.CircleRadius * (bCompact ? 0.82f : 1.18f), 150.f, 1400.f);
			Decision.CircleLayerRadiusOffset = FMath::Clamp(Decision.CircleLayerRadiusOffset * DepthFactor, 80.f, 420.f);
			Decision.CircleMinAnglePerMemberDeg = FMath::Clamp(Decision.CircleMinAnglePerMemberDeg * (bCompact ? 0.92f : 1.10f), 18.f, 80.f);
		}
		else if (Decision.FormationType == EFormationType::Line)
		{
			Decision.LineMinSpacing = FMath::Clamp(Decision.LineMinSpacing * TightFactor, 90.f, 260.f);
			Decision.LineMaxLineLength = FMath::Clamp(Decision.LineMaxLineLength * (bCompact ? 0.85f : 1.25f), 300.f, 20000.f);
			Decision.LineRowOffset = FMath::Clamp(Decision.LineRowOffset * DepthFactor, 100.f, 420.f);
		}
		else if (Decision.FormationType == EFormationType::Wedge)
		{
			Decision.WedgeDesiredDistanceToTarget = FMath::Clamp(Decision.WedgeDesiredDistanceToTarget * DistanceFactor, 500.f, 1000.f);
			Decision.WedgeSideSpacing = FMath::Clamp(Decision.WedgeSideSpacing * DepthFactor, 110.f, 320.f);
			Decision.WedgeDepthSpacing = FMath::Clamp(Decision.WedgeDepthSpacing * DepthFactor, 140.f, 420.f);
			Decision.WedgeLeaderOffset = FMath::Clamp(Decision.WedgeLeaderOffset * DistanceFactor, 220.f, 420.f);
		}
	}
}

void UGroupCommandComponent::OnMemberStartedFight(AActor *NewEnemy)
{
	if (!NewEnemy)
		return;

	AlertGroup(NewEnemy);
}

void UGroupCommandComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetFormationAttackRelease();

	Super::EndPlay(EndPlayReason);

	// Clear timers
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FormationUpdateTimer);
		GetWorld()->GetTimerManager().ClearTimer(AIDecisionUpdateTimer);
	}

	// Unbind from known enemies
	for (AActor *E : KnownEnemies)
	{
		if (!IsValid(E))
			continue;
		if (UHealthComponent *HC = E->FindComponentByClass<UHealthComponent>())
		{
			HC->ActorDied.RemoveDynamic(this, &UGroupCommandComponent::OnKnownEnemyDied);
		}
	}

	// Unbind member delegates
	for (AEnemyCore *M : GroupMembers)
	{
		if (!IsValid(M))
			continue;
		if (M->OwnerTargetComp)
		{
			M->OwnerTargetComp->OnNewEnemy.RemoveDynamic(this, &UGroupCommandComponent::OnMemberStartedFight);
		}
		if (UHealthComponent *HC = M->GetHealthComponent())
		{
			HC->ActorDied.RemoveDynamic(this, &UGroupCommandComponent::OnMemberDiedOrDestroyed);
		}
		M->OnDestroyed.RemoveDynamic(this, &UGroupCommandComponent::OnMemberDiedOrDestroyed);

		// clear owning group pointer on each member
		M->OwningGroupCommandComponent = nullptr;
	}

	KnownEnemies.Empty();
	GroupMembers.Empty();
	MemberFormationIndex.Empty();
	LastKnownEnemyHealth.Empty();
	LastMemberHealth.Empty();
}

void UGroupCommandComponent::AlertGroup(AActor *EnemyActor)
{
	if (!Leader || !IsValid(EnemyActor))
		return;

	if (KnownEnemies.Contains(EnemyActor))
		return;

	KnownEnemies.Add(EnemyActor);

	// Subscribe to the enemy's death and health change so we can remove it from known list,
	// track damage dealt and trigger end-of-combat learning when the last enemy dies.
	if (UHealthComponent *EnemyHealth = EnemyActor->FindComponentByClass<UHealthComponent>())
	{
		EnemyHealth->ActorDied.AddDynamic(this, &UGroupCommandComponent::OnKnownEnemyDied);
		// snapshot initial health
		LastKnownEnemyHealth.Add(EnemyActor, EnemyHealth->GetCurrentHealth());
	}

	// Start combat timers/aggregation if this is the first known enemy
	if (!bGroupInCombat)
	{
		bGroupInCombat = true;
		bCombatLearningSubmitted = false;
		CombatStartTime = FPlatformTime::Seconds();
		AccumulatedDamageDealt = 0.f;
		AccumulatedDamageTaken = 0.f;
	}

	for (AEnemyCore *Member : GroupMembers)
	{
		if (!IsValid(Member) || !Member->OwnerTargetComp)
			continue;

		ACoreAIController *MemberController = Cast<ACoreAIController>(Member->GetController());
		if (!MemberController)
			continue;

		MemberController->CheckSensedActor(EnemyActor);
	}

	GetWorld()->GetTimerManager().SetTimer(
		FormationUpdateTimer,
		this,
		&UGroupCommandComponent::UpdateFormation,
		0.05f,
		true);

	GetWorld()->GetTimerManager().SetTimer(
		AIDecisionUpdateTimer,
		this,
		&UGroupCommandComponent::UpdateTacticalDecision,
		3.0f,
		true);
}

void UGroupCommandComponent::AddMember(AEnemyCore *NewMember)
{
	if (!NewMember)
		return;

	if (GroupMembers.Contains(NewMember))
		return;

	UHealthComponent *HealthComp = NewMember->GetHealthComponent();

	if (!NewMember->OwnerTargetComp || !HealthComp)
	{
		UE_LOG(LogTemp, Error, TEXT("Enemy %s does not have an OwnerTargetComp or HealthComponent - cannot be added to group"), *NewMember->GetName());

		return;
	}

	GroupMembers.Add(NewMember);
	int32 NewIndex = GroupMembers.Num() - 1;
	MemberFormationIndex.Add(NewMember, NewIndex);

	// record owning group on the member so its controller can check membership later
	NewMember->OwningGroupCommandComponent = this;
	NewMember->SetCurrentFormationRole(GetPreferredRoleForMember(NewMember, Leader));

	NewMember->OwnerTargetComp->OnNewEnemy.AddDynamic(this, &UGroupCommandComponent::OnMemberStartedFight);
	HealthComp->ActorDied.AddDynamic(this, &UGroupCommandComponent::OnMemberDiedOrDestroyed);

	NewMember->OnDestroyed.AddDynamic(this, &UGroupCommandComponent::OnMemberDiedOrDestroyed);

	// snapshot initial member health
	LastMemberHealth.Add(NewMember, HealthComp->GetCurrentHealth());

	// Enable formation control on this member's controller
	if (ACoreAIController *NewCtrl = Cast<ACoreAIController>(NewMember->GetController()))
	{
		NewCtrl->SetUnderCommand(true);
		NewCtrl->SetShouldAttackInFormation(false);
	}

	// If we now have more than just the leader, ensure leader is also marked under command
	if (GroupMembers.Num() > 1 && Leader && IsValid(Leader))
	{
		if (ACoreAIController *LeaderCtrl = Cast<ACoreAIController>(Leader->GetController()))
		{
			LeaderCtrl->SetUnderCommand(true);
		}
	}
}

void UGroupCommandComponent::SetUpComponent(AEnemyCore *NewLeader, TArray<FName> DefaultEnemiesToCommand)
{
	if (!NewLeader || DefaultEnemiesToCommand.IsEmpty())
		return;

	TacticalAI = NewObject<UEnemyGroupTacticalAI>(this);
	// CurrentFormation = NewObject<UEnemyFormation_Line>(this);
	CurrentFormation = NewObject<UEnemyFormation_Circle>(this);

	Registry = GetWorld()->GetSubsystem<URegistrySubsystem>();
	if (!Registry)
		return;

	DefaultMemberIDs = DefaultEnemiesToCommand;
	LateDefaultMembersToAdd.Empty();

	Leader = NewLeader;

	AddMember(Leader);

	// Ensure leader controller is marked under command when the group is set up
	if (Leader && IsValid(Leader))
	{
		if (ACoreAIController *LeaderCtrl = Cast<ACoreAIController>(Leader->GetController()))
		{
			LeaderCtrl->SetUnderCommand(true);
		}
	}

	if (Registry->bResetEnded)
	{
		SetUpDefaultSquad();
		return;
	}

	Registry->OnLevelResetEnded.AddDynamic(this, &UGroupCommandComponent::SetUpDefaultSquad);
}

void UGroupCommandComponent::SetFormationTestingOverrides(
	bool bEnableOverrides,
	bool bDisableTacticalAI,
	bool bForceFormation,
	EFormationType ForcedFormation,
	EFormationTuningPreset ForcedPreset,
	bool bSuppressLearning)
{
	bLeaderFormationTestOverridesEnabled = bEnableOverrides;
	bDisableTacticalFormationAIForTesting = bDisableTacticalAI;
	bForceFormationForTesting = bForceFormation;
	ForcedFormationForTesting = ForcedFormation == EFormationType::None ? EFormationType::Circle : ForcedFormation;
	ForcedFormationTuningPreset = ForcedPreset;
	bSuppressTacticalLearningWhileTesting = bSuppressLearning;
}

void UGroupCommandComponent::SetUpDefaultSquad()
{
	if (!Registry)
		return;
	for (const FName &SpawnerID : DefaultMemberIDs)
	{
		AEnemySpawner *Spawner = Registry->GetSpawnerByID(SpawnerID);
		if (!IsValid(Spawner))
		{
			LateDefaultMembersToAdd.AddUnique(SpawnerID);

			Registry->OnEnemySpawned.AddUniqueDynamic(
				this,
				&UGroupCommandComponent::OnRegistryEnemySpawned);
			continue;
		}

		AEnemyCore *Enemy = Spawner->GetSpawnedEnemy();

		if (Enemy == Leader)
			continue;

		if (!IsValid(Enemy))
		{
			LateDefaultMembersToAdd.AddUnique(SpawnerID);

			Registry->OnEnemySpawned.AddUniqueDynamic(
				this,
				&UGroupCommandComponent::OnRegistryEnemySpawned);
			continue;
		}

		AddMember(Enemy);
	}

	Registry->OnLevelResetEnded.RemoveDynamic(this, &UGroupCommandComponent::SetUpDefaultSquad);
}

void UGroupCommandComponent::OnMemberDiedOrDestroyed(AActor *Member)
{
	AEnemyCore *Enemy = Cast<AEnemyCore>(Member);
	if (!Enemy)
		return;

	if (!GroupMembers.Contains(Enemy))
		return;

	SetMemberShouldAttackInFormation(Enemy, false);
	FormationAttackReleaseExpiry.Remove(Enemy);

	if (Enemy->OwnerTargetComp)
	{
		Enemy->OwnerTargetComp->OnNewEnemy.RemoveDynamic(this, &UGroupCommandComponent::OnMemberStartedFight);
	}

	if (UHealthComponent *HealthComp = Enemy->GetHealthComponent())
	{
		HealthComp->ActorDied.RemoveDynamic(this, &UGroupCommandComponent::OnMemberDiedOrDestroyed);
		LastMemberHealth.Remove(Enemy);
	}

	Enemy->OnDestroyed.RemoveDynamic(
		this,
		&UGroupCommandComponent::OnMemberDiedOrDestroyed);

	// clear owning group pointer on the member
	Enemy->OwningGroupCommandComponent = nullptr;

	GroupMembers.Remove(Enemy);
	MemberFormationIndex.Remove(Enemy);

	RebuildFormationIndices();
	if (CurrentFormation)
	{
		CurrentFormation->ResetFormation();
	}

	UpdateFormation();

	// If no members remain, treat as group defeat and notify TacticalAI
	if (GroupMembers.Num() == 0)
	{
		ResetFormationAttackRelease();
		bGroupInCombat = false;
		GetWorld()->GetTimerManager().ClearTimer(FormationUpdateTimer);
		GetWorld()->GetTimerManager().ClearTimer(AIDecisionUpdateTimer);

		if (TacticalAI)
		{
			// mark controllers as not under command (leader probably dead)
			if (Leader && IsValid(Leader))
			{
				if (ACoreAIController *LeaderCtrl = Cast<ACoreAIController>(Leader->GetController()))
				{
					LeaderCtrl->SetUnderCommand(false);
				}
			}

			SubmitCombatLearningResult(ECombatOutcome::Defeat, AccumulatedDamageDealt, AccumulatedDamageTaken);
		}
	}
	else if (GroupMembers.Num() == 1 && GroupMembers[0] == Leader)
	{
		// Only leader is left: stop group logic and mark leader as no longer under command.
		ResetFormationAttackRelease();
		bGroupInCombat = false;
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(FormationUpdateTimer);
			GetWorld()->GetTimerManager().ClearTimer(AIDecisionUpdateTimer);
		}

		if (Leader && IsValid(Leader))
		{
			if (ACoreAIController *LeaderCtrl = Cast<ACoreAIController>(Leader->GetController()))
			{
				LeaderCtrl->SetUnderCommand(false);
			}
		}

		// Notify TacticalAI: treat as a loss but slightly better than full defeat for learning
		if (TacticalAI)
		{
			float AdjDealt = AccumulatedDamageDealt * 1.1f; // slightly credit dealt damage
			float AdjTaken = AccumulatedDamageTaken * 0.9f; // slightly reduce taken damage impact
			SubmitCombatLearningResult(ECombatOutcome::LeaderOnly, AdjDealt, AdjTaken);
		}
	}
}

void UGroupCommandComponent::OnKnownEnemyDied(AActor *DeadEnemy)
{
	if (!DeadEnemy)
		return;

	// Unbind delegate on the dead enemy if possible
	if (UHealthComponent *HC = DeadEnemy->FindComponentByClass<UHealthComponent>())
	{
		HC->ActorDied.RemoveDynamic(this, &UGroupCommandComponent::OnKnownEnemyDied);
		LastKnownEnemyHealth.Remove(DeadEnemy);
	}

	KnownEnemies.Remove(DeadEnemy);

	// If no known enemies remain, stop combat and notify TacticalAI
	if (KnownEnemies.Num() == 0)
	{
		ResetFormationAttackRelease();
		bGroupInCombat = false;

		GetWorld()->GetTimerManager().ClearTimer(FormationUpdateTimer);
		GetWorld()->GetTimerManager().ClearTimer(AIDecisionUpdateTimer);

		if (TacticalAI)
		{
			// mark all members as not under command and treat as victory
			for (AEnemyCore *M : GroupMembers)
			{
				if (!IsValid(M))
					continue;
				if (ACoreAIController *C = Cast<ACoreAIController>(M->GetController()))
				{
					C->SetUnderCommand(false);
				}
			}

			// Use the applied formation as label and treat as victory
			SubmitCombatLearningResult(ECombatOutcome::Victory, AccumulatedDamageDealt, AccumulatedDamageTaken);
		}
	}
}

// Removed dynamic fight/health handlers; damage is sampled during UpdateFormation instead.

void UGroupCommandComponent::OnRegistryEnemySpawned(AEnemyCore *Enemy, FName SpawnerID)
{
	if (!IsValid(Enemy) || !Registry)
		return;
	if (!LateDefaultMembersToAdd.Contains(SpawnerID))
		return;

	AddMember(Enemy);

	LateDefaultMembersToAdd.Remove(SpawnerID);

	if (LateDefaultMembersToAdd.IsEmpty())
	{
		Registry->OnEnemySpawned.RemoveDynamic(
			this,
			&UGroupCommandComponent::OnRegistryEnemySpawned);
	}
}

FVector UGroupCommandComponent::GetFormationLocation(AEnemyCore *Member)
{
	if (!CurrentFormation || !Leader || KnownEnemies.IsEmpty())
		return FVector::ZeroVector;

	int32 *IndexPtr = MemberFormationIndex.Find(Member);
	if (!IndexPtr)
		return FVector::ZeroVector;

	int32 Index = *IndexPtr;

	int32 Total = GroupMembers.Num();

	const FGroupTargetSelection TargetSelection = SelectFormationTarget(this, CurrentFormation, Leader, KnownEnemies);
	if (!TargetSelection.TargetActor)
		return FVector::ZeroVector;

	CurrentFormation->SetTargetLocationOverride(TargetSelection.TargetLocation);

	return CurrentFormation->GetLocationForMember(Member, Index, Total, TargetSelection.TargetActor);
}

void UGroupCommandComponent::ApplyPositionBasedSlotCorrection(
	const TArray<AEnemyCore *> &OrderedMembers,
	const TArray<EFormationMemberRole> &OrderedRoles,
	const TArray<FVector> &OriginalPositions,
	TArray<FVector> &InOutPositions) const
{
	if (bUseClassicFormationPathing)
		return;
	if (!bEnablePositionBasedSlotCorrection)
		return;
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	const int32 Count = FMath::Min(FMath::Min(OrderedMembers.Num(), OrderedRoles.Num()), FMath::Min(OriginalPositions.Num(), InOutPositions.Num()));
	if (Count < 2)
		return;

	FPBMADSlotCorrectionSettings Settings;
	Settings.bProjectCorrectedSlotsToNavMesh = bProjectCorrectedSlotsToNavMesh;
	Settings.bEnablePredictiveSlotAvoidance = bEnablePredictiveSlotAvoidance;
	Settings.bEnableSlotCorrectionCohesion = bEnableSlotCorrectionCohesion;
	Settings.bEnableRoleAwareSlotCorrection = bEnableRoleAwareSlotCorrection;
	Settings.bDrawDebug = bDrawSlotCorrectionDebug;
	Settings.bDrawRadii = bDrawSlotCorrectionRadii;
	Settings.bDrawPredictionLinks = bDrawSlotCorrectionPredictionLinks;
	Settings.Iterations = SlotCorrectionIterations;
	Settings.MinDistance = FMath::Max(0.f, SlotCorrectionMinDistance);
	Settings.Strength = FMath::Clamp(SlotCorrectionStrength, 0.f, 1.f);
	Settings.MaxOffset = FMath::Max(0.f, SlotCorrectionMaxOffset);
	Settings.PredictionTime = FMath::Max(0.05f, SlotCorrectionPredictionTime);
	Settings.PreferredVelocityBlend = FMath::Clamp(SlotCorrectionPreferredVelocityBlend, 0.f, 1.f);
	Settings.RadiusScale = FMath::Max(0.1f, SlotCorrectionRadiusScale);
	Settings.ShapePreservationStrength = FMath::Clamp(SlotCorrectionShapePreservationStrength, 0.f, 1.f);
	Settings.CohesionStrength = FMath::Clamp(SlotCorrectionCohesionStrength, 0.f, 0.5f);

	TArray<FPBMADSlotCorrectionMember> SolverMembers;
	SolverMembers.Reserve(Count);
	for (int32 i = 0; i < Count; ++i)
	{
		AEnemyCore *Member = OrderedMembers[i];

		FPBMADSlotCorrectionMember SolverMember;
		SolverMember.Member = Member;
		SolverMember.CurrentLocation = IsValid(Member) ? Member->GetActorLocation() : OriginalPositions[i];
		SolverMember.CurrentVelocity = IsValid(Member) ? Member->GetVelocity() : FVector::ZeroVector;
		SolverMember.OriginalPosition = OriginalPositions[i];
		SolverMember.CorrectedPosition = InOutPositions[i];
		SolverMember.Role = IsValid(Member) && Member == Leader ? EFormationMemberRole::Leader : OrderedRoles[i];
		SolverMember.bIsLeader = IsValid(Member) && Member == Leader;
		SolverMembers.Add(SolverMember);
	}

	FFormationSlotCorrectionSolver::CorrectSlots(GetWorld(), Settings, SolverMembers);

	for (int32 i = 0; i < Count; ++i)
	{
		InOutPositions[i] = SolverMembers[i].CorrectedPosition;
	}

	if (bDrawSlotCorrectionDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("SlotCorrection: PBMAD corrected %d slots Iterations=%d MinDistance=%.1f Strength=%.2f MaxOffset=%.1f Predictive=%d Cohesion=%d"),
			   Count,
			   Settings.Iterations,
			   Settings.MinDistance,
			   Settings.Strength,
			   Settings.MaxOffset,
			   (int)Settings.bEnablePredictiveSlotAvoidance,
			   (int)Settings.bEnableSlotCorrectionCohesion);
	}
}

void UGroupCommandComponent::ApplyFormationParameters(const FFormationDecision &Decision)
{
	bool bChangedFormationType = false;

	if (Decision.FormationType == EFormationType::Line)
	{
		UEnemyFormation_Line *LineForm = Cast<UEnemyFormation_Line>(CurrentFormation);
		if (!LineForm)
		{
			CurrentFormation = NewObject<UEnemyFormation_Line>(this);
			LineForm = Cast<UEnemyFormation_Line>(CurrentFormation);
			bChangedFormationType = true;
		}

		if (LineForm && Decision.bSetLineParams)
		{
			LineForm->MinSpacing = Decision.LineMinSpacing;
			LineForm->MaxLineLength = Decision.LineMaxLineLength;
			LineForm->RowOffset = Decision.LineRowOffset;
		}
	}
	else if (Decision.FormationType == EFormationType::Circle)
	{
		UEnemyFormation_Circle *CircleForm = Cast<UEnemyFormation_Circle>(CurrentFormation);
		if (!CircleForm)
		{
			CurrentFormation = NewObject<UEnemyFormation_Circle>(this);
			CircleForm = Cast<UEnemyFormation_Circle>(CurrentFormation);
			bChangedFormationType = true;
		}

		if (CircleForm && Decision.bSetCircleParams)
		{
			CircleForm->Radius = Decision.CircleRadius;
			CircleForm->ArcAngleDegrees = Decision.CircleArcAngleDegrees;
			CircleForm->ArcOffsetDegrees = Decision.CircleArcOffsetDegrees;
			CircleForm->MinAnglePerMemberDeg = Decision.CircleMinAnglePerMemberDeg;
			CircleForm->LayerRadiusOffset = Decision.CircleLayerRadiusOffset;
		}
	}
	else if (Decision.FormationType == EFormationType::Wedge)
	{
		UEnemyFormation_Wedge *WedgeForm = Cast<UEnemyFormation_Wedge>(CurrentFormation);
		if (!WedgeForm)
		{
			CurrentFormation = NewObject<UEnemyFormation_Wedge>(this);
			WedgeForm = Cast<UEnemyFormation_Wedge>(CurrentFormation);
			bChangedFormationType = true;
		}

		if (WedgeForm && Decision.bSetWedgeParams)
		{
			WedgeForm->DesiredDistanceToTarget = Decision.WedgeDesiredDistanceToTarget;
			WedgeForm->SideSpacing = Decision.WedgeSideSpacing;
			WedgeForm->DepthSpacing = Decision.WedgeDepthSpacing;
			WedgeForm->LeaderOffset = Decision.WedgeLeaderOffset;
		}
	}

	if (CurrentFormation && bChangedFormationType)
	{
		CurrentFormation->ResetFormation();
	}
}

float UGroupCommandComponent::EstimateDesiredFormationWidth(const FFormationDecision &Decision, int32 ActiveMemberCount) const
{
	const int32 FormationMemberCount = FMath::Max(1, ActiveMemberCount - (IsValid(Leader) ? 1 : 0));

	if (Decision.FormationType == EFormationType::Circle)
	{
		const float ArcAngle = FMath::Clamp(Decision.CircleArcAngleDegrees, Decision.CircleMinAnglePerMemberDeg, 360.f);
		const float AnglePerMember = FMath::Max(Decision.CircleMinAnglePerMemberDeg, 1.f);
		const int32 SlotsPerLayer = FMath::Max(1, FMath::CeilToInt(ArcAngle / AnglePerMember));
		const int32 LayerCount = FMath::Max(1, FMath::CeilToInt((float)FormationMemberCount / (float)SlotsPerLayer));
		const float OuterRadius = Decision.CircleRadius + FMath::Max(0, LayerCount - 1) * Decision.CircleLayerRadiusOffset;
		return FMath::Max(OuterRadius * 2.f, Decision.CircleRadius);
	}

	if (Decision.FormationType == EFormationType::Line)
	{
		const float DesiredRowWidth = FMath::Max(Decision.LineMinSpacing, FormationMemberCount * Decision.LineMinSpacing);
		return FMath::Max(Decision.LineMinSpacing, FMath::Min(Decision.LineMaxLineLength, DesiredRowWidth));
	}

	if (Decision.FormationType == EFormationType::Wedge)
	{
		const int32 PairCount = FMath::Max(1, FMath::CeilToInt((float)FormationMemberCount * 0.5f));
		return FMath::Max(Decision.WedgeSideSpacing, PairCount * Decision.WedgeSideSpacing * 2.f);
	}

	return FMath::Max(1.f, SlotCorrectionMinDistance * FMath::Max(1, FormationMemberCount));
}

FFormationDecision UGroupCommandComponent::AdaptFormationDecisionForEnvironment(
	const FFormationDecision &BaseDecision,
	const FFormationEnvironmentAnalysisResult &Environment,
	int32 ActiveMemberCount) const
{
	if (!bEnableEnvironmentAwareFormationAdaptation || !Environment.bValid || Environment.Mode == EFormationEnvironmentMode::Open)
	{
		return BaseDecision;
	}

	FFormationDecision AdaptedDecision = BaseDecision;
	const float WidthScale = FMath::Clamp(Environment.WidthScale, 0.1f, 1.f);
	const float LimitedScale = FMath::Clamp(WidthScale, EnvironmentLimitedCompressionMin, 1.f);
	const int32 FormationMemberCount = FMath::Max(1, ActiveMemberCount - (IsValid(Leader) ? 1 : 0));
	const bool bMustUseColumn =
		Environment.Mode == EFormationEnvironmentMode::NarrowCorridor ||
		(Environment.AvailableWidth > KINDA_SMALL_NUMBER &&
		 Environment.AvailableWidth <= EnvironmentNarrowForceLineWidth &&
		 FormationMemberCount >= EnvironmentLargeGroupForceLineCount);

	if (bMustUseColumn)
	{
		const float BaseLineSpacing = FMath::Max(90.f, BaseDecision.LineMinSpacing);
		const float UsableWidth = Environment.AvailableWidth > KINDA_SMALL_NUMBER
									  ? FMath::Max(90.f, Environment.AvailableWidth * FMath::Clamp(EnvironmentWidthSafetyMargin, 0.1f, 1.f))
									  : FMath::Min(BaseLineSpacing, EnvironmentNarrowForceLineWidth);
		const float WidthLimitedSpacing = FMath::Clamp(UsableWidth * 0.45f, 90.f, BaseLineSpacing);
		const float DepthSpacing = FMath::Max(BaseDecision.LineRowOffset, WidthLimitedSpacing * 1.65f);

		AdaptedDecision.FormationType = EFormationType::Line;
		AdaptedDecision.bSetLineParams = true;
		AdaptedDecision.LineMinSpacing = WidthLimitedSpacing;
		AdaptedDecision.LineMaxLineLength = FMath::Min(EnvironmentNarrowForceLineWidth, FMath::Max(WidthLimitedSpacing * 1.25f, WidthLimitedSpacing + 1.f));
		AdaptedDecision.LineRowOffset = FMath::Max(150.f, DepthSpacing);
		AdaptedDecision.bSetCircleParams = BaseDecision.bSetCircleParams;
		AdaptedDecision.bSetWedgeParams = BaseDecision.bSetWedgeParams;

		return AdaptedDecision;
	}

	if (Environment.Mode == EFormationEnvironmentMode::Limited)
	{
		if (AdaptedDecision.FormationType == EFormationType::Circle)
		{
			AdaptedDecision.bSetCircleParams = true;
			const float WidthLimitedRadius = Environment.AvailableWidth > KINDA_SMALL_NUMBER
												 ? Environment.AvailableWidth * FMath::Clamp(EnvironmentWidthSafetyMargin, 0.1f, 1.f) * 0.45f
												 : BaseDecision.CircleRadius * LimitedScale;
			AdaptedDecision.CircleRadius = FMath::Max(140.f, FMath::Min(BaseDecision.CircleRadius * LimitedScale * 0.8f, WidthLimitedRadius));
			AdaptedDecision.CircleLayerRadiusOffset = FMath::Max(90.f, FMath::Min(BaseDecision.CircleLayerRadiusOffset * LimitedScale * 0.65f, AdaptedDecision.CircleRadius * 0.6f));
			AdaptedDecision.CircleArcAngleDegrees = FMath::Min(BaseDecision.CircleArcAngleDegrees, FMath::Lerp(90.f, 150.f, LimitedScale));
		}
		else if (AdaptedDecision.FormationType == EFormationType::Line)
		{
			AdaptedDecision.bSetLineParams = true;
			AdaptedDecision.LineMinSpacing = FMath::Max(95.f, BaseDecision.LineMinSpacing * LimitedScale);
			const float WidthLimitedLength = Environment.AvailableWidth > KINDA_SMALL_NUMBER
												 ? Environment.AvailableWidth * FMath::Clamp(EnvironmentWidthSafetyMargin, 0.1f, 1.f)
												 : BaseDecision.LineMaxLineLength * LimitedScale;
			AdaptedDecision.LineMaxLineLength = FMath::Max(
				AdaptedDecision.LineMinSpacing * 1.25f,
				FMath::Min(BaseDecision.LineMaxLineLength * LimitedScale, WidthLimitedLength));
			AdaptedDecision.LineRowOffset = FMath::Max(140.f, BaseDecision.LineRowOffset * FMath::Max(0.85f, LimitedScale));
		}
		else if (AdaptedDecision.FormationType == EFormationType::Wedge)
		{
			AdaptedDecision.bSetWedgeParams = true;
			AdaptedDecision.WedgeSideSpacing = FMath::Max(100.f, BaseDecision.WedgeSideSpacing * LimitedScale);
			AdaptedDecision.WedgeDepthSpacing = FMath::Max(BaseDecision.WedgeDepthSpacing, BaseDecision.WedgeDepthSpacing * (1.f + (1.f - LimitedScale) * 0.35f));
		}

		return AdaptedDecision;
	}

	return AdaptedDecision;
}

void UGroupCommandComponent::LogEnvironmentAdaptationIfNeeded(const FFormationEnvironmentAnalysisResult &Environment)
{
	if (!bLogEnvironmentAdaptation && (!bDrawEnvironmentAdaptationDebug || (bHasLoggedEnvironmentMode && LastLoggedEnvironmentMode == Environment.Mode)))
	{
		return;
	}

	if (!bHasLoggedEnvironmentMode || LastLoggedEnvironmentMode != Environment.Mode || bLogEnvironmentAdaptation)
	{
		UE_LOG(LogTemp, Warning, TEXT("FormationEnvironment: Mode=%s Width=%.1f Depth=%.1f Scale=%.2f"),
			   GetFormationEnvironmentModeName(Environment.Mode),
			   Environment.AvailableWidth,
			   Environment.AvailableDepth,
			   Environment.WidthScale);
		LastLoggedEnvironmentMode = Environment.Mode;
		bHasLoggedEnvironmentMode = true;
	}
}

void UGroupCommandComponent::ClampFormationSlotsToEnvironmentWidth(TArray<FVector> &InOutPositions) const
{
	if (bUseClassicFormationPathing)
		return;
	if (!bEnableEnvironmentAwareFormationAdaptation || !bClampFormationSlotsToEnvironmentWidth)
		return;
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	if (!CachedEnvironmentAnalysis.bValid || CachedEnvironmentAnalysis.Mode == EFormationEnvironmentMode::Open)
		return;
	if (InOutPositions.IsEmpty())
		return;

	const float UsableWidth = CachedEnvironmentAnalysis.AvailableWidth * FMath::Clamp(EnvironmentWidthSafetyMargin, 0.1f, 1.f);
	if (UsableWidth <= KINDA_SMALL_NUMBER)
		return;

	FVector Forward = FVector::VectorPlaneProject(CachedEnvironmentAnalysis.CorridorDirection, FVector::UpVector).GetSafeNormal();
	if (Forward.IsNearlyZero())
	{
		Forward = FVector::VectorPlaneProject(CachedEnvironmentAnalysis.Forward, FVector::UpVector).GetSafeNormal();
	}
	if (Forward.IsNearlyZero())
	{
		Forward = FVector::ForwardVector;
	}

	const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
	if (Right.IsNearlyZero())
		return;

	const FVector Center = CachedEnvironmentAnalysis.Origin;
	const float MaxLateral = UsableWidth * 0.5f;
	UNavigationSystemV1 *NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	for (FVector &SlotLocation : InOutPositions)
	{
		const FVector OriginalLocation = SlotLocation;
		FVector Offset = SlotLocation - Center;
		Offset.Z = 0.f;

		const float Lateral = FVector::DotProduct(Offset, Right);
		const float ClampedLateral = FMath::Clamp(Lateral, -MaxLateral, MaxLateral);
		if (!FMath::IsNearlyEqual(Lateral, ClampedLateral, 1.f))
		{
			SlotLocation -= Right * (Lateral - ClampedLateral);
		}

		if (NavSystem)
		{
			FNavLocation ProjectedLocation;
			if (NavSystem->ProjectPointToNavigation(SlotLocation, ProjectedLocation, EnvironmentProbeProjectionExtent))
			{
				SlotLocation = ProjectedLocation.Location;
			}
		}

		if (bDrawEnvironmentAdaptationDebug)
		{
			const FColor Color = CachedEnvironmentAnalysis.Mode == EFormationEnvironmentMode::NarrowCorridor ? FColor::Orange : FColor::Yellow;
			DrawDebugLine(GetWorld(), OriginalLocation, SlotLocation, Color, false, 1.f, 0, 2.f);
			DrawDebugSphere(GetWorld(), SlotLocation, 14.f, 8, Color, false, 1.f);
		}
	}
}

FFormationDecision UGroupCommandComponent::BuildEnvironmentAdaptedDecision(
	const FFormationDecision &BaseDecision,
	const TArray<AEnemyCore *> &ActiveMembers,
	const FVector &TargetLocation,
	bool bForceRefresh)
{
	if (bUseClassicFormationPathing)
	{
		CurrentEnvironmentAdaptedDecision = BaseDecision;
		return BaseDecision;
	}

	if (!bEnableEnvironmentAwareFormationAdaptation || !GetOwner() || !GetOwner()->HasAuthority())
	{
		CurrentEnvironmentAdaptedDecision = BaseDecision;
		return BaseDecision;
	}

	const FVector GroupCenter = GetActiveMembersCenter(ActiveMembers);
	if (ActiveMembers.IsEmpty())
	{
		CurrentEnvironmentAdaptedDecision = BaseDecision;
		return BaseDecision;
	}

	FVector AnalysisForward = FVector::VectorPlaneProject(TargetLocation - GroupCenter, FVector::UpVector).GetSafeNormal();
	if (AnalysisForward.IsNearlyZero() && IsValid(Leader))
	{
		AnalysisForward = FVector::VectorPlaneProject(TargetLocation - Leader->GetActorLocation(), FVector::UpVector).GetSafeNormal();
	}
	if (AnalysisForward.IsNearlyZero())
	{
		AnalysisForward = FVector::ForwardVector;
	}

	UWorld *World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : FPlatformTime::Seconds();
	const float MoveThresholdSq = FMath::Square(FMath::Max(0.f, EnvironmentAnalysisMoveThreshold));
	const bool bCacheExpired = (Now - LastEnvironmentAnalysisTime) >= FMath::Max(0.1f, EnvironmentAnalysisInterval);
	const bool bMovedEnough = FVector::DistSquared2D(GroupCenter, LastEnvironmentAnalysisOrigin) > MoveThresholdSq ||
							  FVector::DistSquared2D(AnalysisForward, LastEnvironmentAnalysisForward) > 0.05f;
	const bool bNeedRefresh = bForceRefresh || !CachedEnvironmentAnalysis.bValid || bCacheExpired || bMovedEnough;

	if (bNeedRefresh)
	{
		FFormationEnvironmentAnalyzerSettings Settings;
		Settings.ProbeStep = EnvironmentProbeStep;
		Settings.MaxProbeDistance = EnvironmentMaxProbeDistance;
		Settings.ProjectionExtent = EnvironmentProbeProjectionExtent;
		Settings.ProjectionTolerance = EnvironmentProjectionTolerance;
		Settings.LimitedWidthScaleThreshold = LimitedWidthScaleThreshold;
		Settings.NarrowWidthScaleThreshold = NarrowWidthScaleThreshold;
		Settings.bDrawDebug = bDrawEnvironmentAdaptationDebug;

		FFormationEnvironmentAnalysisInput GroupInput;
		GroupInput.Origin = GroupCenter;
		GroupInput.Forward = AnalysisForward;
		GroupInput.DesiredFormationWidth = EstimateDesiredFormationWidth(BaseDecision, ActiveMembers.Num());
		GroupInput.FormationType = BaseDecision.FormationType;

		Settings.DebugLabel = TEXT("Group");
		const FFormationEnvironmentAnalysisResult GroupAnalysis = FFormationEnvironmentAnalyzer::Analyze(World, Settings, GroupInput);

		FFormationEnvironmentAnalysisInput TargetInput = GroupInput;
		TargetInput.Origin = TargetLocation;

		Settings.DebugLabel = TEXT("Target");
		const FFormationEnvironmentAnalysisResult TargetAnalysis = FFormationEnvironmentAnalyzer::Analyze(World, Settings, TargetInput);

		CachedEnvironmentAnalysis = FFormationEnvironmentAnalyzer::SelectMostRestrictive(GroupAnalysis, TargetAnalysis);
		LastEnvironmentAnalysisTime = Now;
		LastEnvironmentAnalysisOrigin = GroupCenter;
		LastEnvironmentAnalysisForward = AnalysisForward;
		LogEnvironmentAdaptationIfNeeded(CachedEnvironmentAnalysis);

		if (bDrawEnvironmentAdaptationDebug && CachedEnvironmentAnalysis.bValid)
		{
			const FColor SelectedColor = CachedEnvironmentAnalysis.Mode == EFormationEnvironmentMode::Open
											? FColor::Green
											: (CachedEnvironmentAnalysis.Mode == EFormationEnvironmentMode::Limited ? FColor::Yellow : FColor::Red);
			DrawDebugString(
				World,
				CachedEnvironmentAnalysis.Origin + FVector(0.f, 0.f, 180.f),
				FString::Printf(TEXT("Selected %s Width=%.0f"), GetFormationEnvironmentModeName(CachedEnvironmentAnalysis.Mode), CachedEnvironmentAnalysis.AvailableWidth),
				nullptr,
				SelectedColor,
				1.f,
				false);
		}
	}

	CurrentEnvironmentAdaptedDecision = AdaptFormationDecisionForEnvironment(BaseDecision, CachedEnvironmentAnalysis, ActiveMembers.Num());
	return CurrentEnvironmentAdaptedDecision;
}

void UGroupCommandComponent::UpdateFormation()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	if (!CurrentFormation)
		return;
	if (KnownEnemies.Num() == 0)
		return;

	const FGroupTargetSelection TargetSelection = SelectFormationTarget(this, CurrentFormation, Leader, KnownEnemies);
	if (!TargetSelection.TargetActor)
		return;
	AActor *Target = TargetSelection.TargetActor;

	TArray<AEnemyCore *> ActiveMembers;
	ActiveMembers.Reserve(GroupMembers.Num());

	for (AEnemyCore *M : GroupMembers)
	{
		if (IsValid(M))
		{
			ActiveMembers.Add(M);
		}
	}

	const int32 Total = ActiveMembers.Num();

	// Sample health deltas for members and known enemies to accumulate combat damage stats
	for (AEnemyCore *M : ActiveMembers)
	{
		if (!IsValid(M))
			continue;
		if (UHealthComponent *HC = M->GetHealthComponent())
		{
			float Curr = HC->GetCurrentHealth();
			float *PrevPtr = LastMemberHealth.Find(M);
			if (PrevPtr)
			{
				if (*PrevPtr > Curr)
					AccumulatedDamageTaken += (*PrevPtr - Curr);
				*PrevPtr = Curr;
			}
			else
			{
				LastMemberHealth.Add(M, Curr);
			}
		}
	}

	for (AActor *E : KnownEnemies)
	{
		if (!IsValid(E))
			continue;
		if (UHealthComponent *HC = E->FindComponentByClass<UHealthComponent>())
		{
			float Curr = HC->GetCurrentHealth();
			float *PrevPtr = LastKnownEnemyHealth.Find(E);
			if (PrevPtr)
			{
				if (*PrevPtr > Curr)
					AccumulatedDamageDealt += (*PrevPtr - Curr);
				*PrevPtr = Curr;
			}
			else
			{
				LastKnownEnemyHealth.Add(E, Curr);
			}
		}
	}

	if (bHasCurrentFormationDecision)
	{
		const FFormationDecision AdaptedDecision = BuildEnvironmentAdaptedDecision(
			CurrentBaseFormationDecision,
			ActiveMembers,
			TargetSelection.TargetLocation,
			false);
		ApplyFormationParameters(AdaptedDecision);
		CurrentAppliedFormation = AdaptedDecision.FormationType;
		CurrentAppliedTuningPreset = AdaptedDecision.TuningPreset;
	}

	// Provide leader context to formation before updating anchor
	CurrentFormation->SetTargetLocationOverride(TargetSelection.TargetLocation);
	CurrentFormation->SetLeader(Leader);
	CurrentFormation->UpdateFormationAnchor(Leader, Target, ActiveMembers);

	TArray<FVector> DesiredPositions;
	DesiredPositions.SetNumZeroed(Total);
	for (int32 i = 0; i < Total; i++)
	{
		AEnemyCore *Member = ActiveMembers[i];

		if (!Member)
			continue;

		DesiredPositions[i] = CurrentFormation->GetLocationForMember(
			Member,
			i,
			Total,
			Target);
	}

	ClampFormationSlotsToEnvironmentWidth(DesiredPositions);

	ApplyMinimalTravelReindexing(ActiveMembers, DesiredPositions);
	GroupMembers = ActiveMembers;
	RebuildFormationIndices();

	TArray<FVector> CorrectedPositions = DesiredPositions;
	TArray<EFormationMemberRole> ActiveRoles;
	ActiveRoles.SetNum(Total);
	for (int32 i = 0; i < Total; ++i)
	{
		AEnemyCore *Member = ActiveMembers[i];
		ActiveRoles[i] = IsValid(Member) && Member == Leader
							 ? EFormationMemberRole::Leader
							 : (IsValid(Member) ? Member->GetCurrentFormationRole() : EFormationMemberRole::Defender);
	}

	ApplyPositionBasedSlotCorrection(ActiveMembers, ActiveRoles, DesiredPositions, CorrectedPositions);

	UpdateFormationAttackRelease(ActiveMembers, CorrectedPositions, TargetSelection.TargetLocation);

	for (int32 i = 0; i < Total; i++)
	{
		AEnemyCore *Member = ActiveMembers[i];

		if (!Member)
			continue;

		ACoreAIController *Controller = Cast<ACoreAIController>(Member->GetController());
		if (!Controller)
			continue;

		Controller->SetFormationLocation(CorrectedPositions[i]);
		Controller->SetFormationRole(Member->GetCurrentFormationRole());
	}
}

void UGroupCommandComponent::RebuildFormationIndices()
{
	MemberFormationIndex.Empty();

	for (int32 i = 0; i < GroupMembers.Num(); i++)
	{
		AEnemyCore *Member = GroupMembers[i];
		if (IsValid(Member))
		{
			MemberFormationIndex.Add(Member, i);
		}
	}
}

void UGroupCommandComponent::ApplyMinimalTravelReindexing(TArray<AEnemyCore *> &InOutOrderedMembers, const TArray<FVector> &SlotPositions) const
{
	if (!bUseMinimalTravelReindexing)
		return;

	const int32 Total = FMath::Min(InOutOrderedMembers.Num(), SlotPositions.Num());
	if (Total < 2)
		return;

	TArray<TArray<float>> Cost;
	Cost.SetNum(Total);
	for (int32 i = 0; i < Total; ++i)
	{
		Cost[i].SetNum(Total);
		const FVector MemberLocation = IsValid(InOutOrderedMembers[i])
										   ? InOutOrderedMembers[i]->GetActorLocation()
										   : FVector::ZeroVector;

		for (int32 j = 0; j < Total; ++j)
		{
			Cost[i][j] = FVector::DistSquared2D(MemberLocation, SlotPositions[j]);
		}
	}

	TArray<float> U;
	TArray<float> V;
	TArray<int32> P;
	TArray<int32> Way;
	U.Init(0.f, Total + 1);
	V.Init(0.f, Total + 1);
	P.Init(0, Total + 1);
	Way.Init(0, Total + 1);

	for (int32 i = 1; i <= Total; ++i)
	{
		P[0] = i;
		int32 J0 = 0;
		TArray<float> MinV;
		TArray<char> Used;
		MinV.Init(TNumericLimits<float>::Max(), Total + 1);
		Used.Init(false, Total + 1);

		do
		{
			Used[J0] = true;
			const int32 I0 = P[J0];
			float Delta = TNumericLimits<float>::Max();
			int32 J1 = 0;

			for (int32 J = 1; J <= Total; ++J)
			{
				if (Used[J])
					continue;

				const float Cur = Cost[I0 - 1][J - 1] - U[I0] - V[J];
				if (Cur < MinV[J])
				{
					MinV[J] = Cur;
					Way[J] = J0;
				}
				if (MinV[J] < Delta)
				{
					Delta = MinV[J];
					J1 = J;
				}
			}

			for (int32 J = 0; J <= Total; ++J)
			{
				if (Used[J])
				{
					U[P[J]] += Delta;
					V[J] -= Delta;
				}
				else
				{
					MinV[J] -= Delta;
				}
			}

			J0 = J1;
		} while (P[J0] != 0);

		do
		{
			const int32 J1 = Way[J0];
			P[J0] = P[J1];
			J0 = J1;
		} while (J0 != 0);
	}

	TArray<int32> MemberAssigned;
	MemberAssigned.Init(-1, Total);
	for (int32 J = 1; J <= Total; ++J)
	{
		if (P[J] > 0)
		{
			MemberAssigned[P[J] - 1] = J - 1;
		}
	}

	const int32 LeaderMemberIndex = InOutOrderedMembers.IndexOfByKey(Leader);
	if (LeaderMemberIndex != INDEX_NONE && MemberAssigned.IsValidIndex(LeaderMemberIndex))
	{
		const int32 LeaderSlotIndex = LeaderMemberIndex;
		const int32 CurrentLeaderSlot = MemberAssigned[LeaderMemberIndex];

		if (CurrentLeaderSlot != LeaderSlotIndex)
		{
			for (int32 M = 0; M < Total; ++M)
			{
				if (M != LeaderMemberIndex && MemberAssigned[M] == LeaderSlotIndex)
				{
					MemberAssigned[M] = CurrentLeaderSlot;
					break;
				}
			}

			MemberAssigned[LeaderMemberIndex] = LeaderSlotIndex;
		}
	}

	TArray<AEnemyCore *> NewOrder;
	NewOrder.SetNumZeroed(Total);
	TArray<bool> bUsedMember;
	bUsedMember.Init(false, Total);

	for (int32 M = 0; M < Total; ++M)
	{
		const int32 SlotIndex = MemberAssigned[M];
		if (SlotIndex >= 0 && SlotIndex < Total && !NewOrder[SlotIndex])
		{
			NewOrder[SlotIndex] = InOutOrderedMembers[M];
			bUsedMember[M] = true;
		}
	}

	int32 FillMemberIndex = 0;
	for (int32 SlotIndex = 0; SlotIndex < Total; ++SlotIndex)
	{
		if (NewOrder[SlotIndex])
			continue;

		while (FillMemberIndex < Total && bUsedMember[FillMemberIndex])
		{
			FillMemberIndex++;
		}

		if (FillMemberIndex < Total)
		{
			NewOrder[SlotIndex] = InOutOrderedMembers[FillMemberIndex];
			bUsedMember[FillMemberIndex] = true;
		}
	}

	InOutOrderedMembers = NewOrder;
}

void UGroupCommandComponent::SetMemberShouldAttackInFormation(AEnemyCore *Member, bool bShouldAttack) const
{
	if (!IsValid(Member))
		return;

	if (Member == Leader)
	{
		bShouldAttack = false;
	}

	if (ACoreAIController *Controller = Cast<ACoreAIController>(Member->GetController()))
	{
		Controller->SetShouldAttackInFormation(bShouldAttack);
	}
}

void UGroupCommandComponent::ResetFormationAttackRelease()
{
	for (AEnemyCore *Member : GroupMembers)
	{
		SetMemberShouldAttackInFormation(Member, false);
	}

	if (IsValid(Leader))
	{
		SetMemberShouldAttackInFormation(Leader, false);
	}

	FormationAttackReleaseExpiry.Empty();
	LastFormationAttackReleaseSelectionTime = -100000.0;
}

void UGroupCommandComponent::UpdateFormationAttackRelease(
	const TArray<AEnemyCore *> &OrderedMembers,
	const TArray<FVector> &SlotPositions,
	const FVector &TargetLocation)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	const int32 Count = FMath::Min(OrderedMembers.Num(), SlotPositions.Num());

	if (IsValid(Leader))
	{
		SetMemberShouldAttackInFormation(Leader, false);
		FormationAttackReleaseExpiry.Remove(Leader);
	}

	for (auto It = FormationAttackReleaseExpiry.CreateIterator(); It; ++It)
	{
		AEnemyCore *ReleasedMember = It.Key();
		if (!IsValid(ReleasedMember) || ReleasedMember == Leader || It.Value() <= Now)
		{
			SetMemberShouldAttackInFormation(ReleasedMember, false);
			It.RemoveCurrent();
		}
	}

	if (!bEnableFormationAttackRelease || !bGroupInCombat || KnownEnemies.IsEmpty() || Count <= 0)
	{
		ResetFormationAttackRelease();
		return;
	}

	TArray<AEnemyCore *> EligibleMembers;
	EligibleMembers.Reserve(Count);
	int32 NearSlotCount = 0;
	const float ReadyDistanceSq = FMath::Square(FMath::Max(0.f, FormationAttackReadyDistance));

	for (int32 i = 0; i < Count; ++i)
	{
		AEnemyCore *Member = OrderedMembers[i];
		if (!IsValid(Member) || Member == Leader)
			continue;

		EligibleMembers.Add(Member);
		if (FVector::DistSquared2D(Member->GetActorLocation(), SlotPositions[i]) <= ReadyDistanceSq)
		{
			NearSlotCount++;
		}
	}

	const int32 NonLeaderCount = EligibleMembers.Num();
	if (NonLeaderCount < FMath::Max(1, FormationAttackReleaseMinNonLeaderCount))
	{
		ResetFormationAttackRelease();
		return;
	}

	const int32 RequiredReadyCount = FMath::Clamp(
		FMath::CeilToInt((float)NonLeaderCount * FMath::Clamp(FormationAttackReadyMemberFraction, 0.f, 1.f)),
		1,
		NonLeaderCount);
	const bool bFormationReadyForAttackRelease = NearSlotCount >= RequiredReadyCount;

	int32 DesiredReleaseCount = FMath::FloorToInt((float)NonLeaderCount * FMath::Clamp(FormationAttackReleasePercent, 0.f, 1.f));
	DesiredReleaseCount = FMath::Clamp(DesiredReleaseCount, 1, FMath::Max(1, FormationAttackReleaseMaxCount));
	DesiredReleaseCount = FMath::Min(DesiredReleaseCount, FMath::Max(0, NonLeaderCount - 1));
	if (DesiredReleaseCount <= 0)
	{
		ResetFormationAttackRelease();
		return;
	}

	for (AEnemyCore *Member : EligibleMembers)
	{
		const bool bIsReleased = FormationAttackReleaseExpiry.Contains(Member);
		SetMemberShouldAttackInFormation(Member, bIsReleased);
	}

	const bool bCanRefreshSelection =
		(Now - LastFormationAttackReleaseSelectionTime) >= FMath::Max(0.5f, FormationAttackRefreshSeconds);
	if (!bFormationReadyForAttackRelease || !bCanRefreshSelection || FormationAttackReleaseExpiry.Num() >= DesiredReleaseCount)
	{
		return;
	}

	TArray<FFormationAttackCandidate> Candidates;
	Candidates.Reserve(EligibleMembers.Num());
	for (AEnemyCore *Member : EligibleMembers)
	{
		if (FormationAttackReleaseExpiry.Contains(Member))
			continue;

		FFormationAttackCandidate Candidate;
		Candidate.Member = Member;
		Candidate.DistSqToTarget = FVector::DistSquared2D(Member->GetActorLocation(), TargetLocation);
		Candidates.Add(Candidate);
	}

	Candidates.Sort([](const FFormationAttackCandidate &A, const FFormationAttackCandidate &B)
	{
		return A.DistSqToTarget < B.DistSqToTarget;
	});

	const double HoldSeconds = FMath::Max(1.0f, FormationAttackHoldSeconds);
	for (const FFormationAttackCandidate &Candidate : Candidates)
	{
		if (FormationAttackReleaseExpiry.Num() >= DesiredReleaseCount)
			break;

		if (!IsValid(Candidate.Member) || Candidate.Member == Leader)
			continue;

		FormationAttackReleaseExpiry.Add(Candidate.Member, Now + HoldSeconds);
		SetMemberShouldAttackInFormation(Candidate.Member, true);
	}

	LastFormationAttackReleaseSelectionTime = Now;
}

void UGroupCommandComponent::SubmitCombatLearningResult(ECombatOutcome Outcome, float DamageDealt, float DamageTaken)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	if (bCombatLearningSubmitted)
		return;
	if (bLeaderFormationTestOverridesEnabled && bSuppressTacticalLearningWhileTesting)
	{
		bCombatLearningSubmitted = true;
		return;
	}
	if (!TacticalAI)
		return;
	if (CurrentAppliedFormation == EFormationType::None)
		return;

	bCombatLearningSubmitted = true;

	const float CombatDurationSeconds = CombatStartTime > 0.0
											? FMath::Max(0.f, (float)(FPlatformTime::Seconds() - CombatStartTime))
											: 0.f;

	TacticalAI->NotifyCombatEnded(
		CurrentAppliedFormation,
		CurrentAppliedTuningPreset,
		Outcome,
		DamageDealt,
		DamageTaken,
		CombatDurationSeconds);
}

FFormationDecision UGroupCommandComponent::BuildTestingFormationDecision() const
{
	FFormationDecision Decision;
	Decision.FormationType = ForcedFormationForTesting == EFormationType::None ? EFormationType::Circle : ForcedFormationForTesting;
	Decision.TuningPreset = ForcedFormationTuningPreset;
	Decision.bEnableBacklineSwap = false;
	Decision.BacklineHpDiffThresholdPercent = 40.f;

	switch (Decision.FormationType)
	{
	case EFormationType::Line:
		Decision.bSetLineParams = true;
		Decision.LineMinSpacing = 150.f;
		Decision.LineMaxLineLength = 10000.f;
		Decision.LineRowOffset = 200.f;
		break;
	case EFormationType::Wedge:
		Decision.bSetWedgeParams = true;
		Decision.WedgeDesiredDistanceToTarget = 700.f;
		Decision.WedgeSideSpacing = 180.f;
		Decision.WedgeDepthSpacing = 220.f;
		Decision.WedgeLeaderOffset = 300.f;
		break;
	case EFormationType::Circle:
	default:
		Decision.FormationType = EFormationType::Circle;
		Decision.bSetCircleParams = true;
		Decision.CircleRadius = 700.f;
		Decision.CircleArcAngleDegrees = 360.f;
		Decision.CircleArcOffsetDegrees = 0.f;
		Decision.CircleMinAnglePerMemberDeg = 40.f;
		Decision.CircleLayerRadiusOffset = 200.f;
		break;
	}

	ApplyTestingTuningPreset(Decision);
	return Decision;
}

void UGroupCommandComponent::UpdateTacticalDecision()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	if (KnownEnemies.IsEmpty())
		return;

	if (bLeaderFormationTestOverridesEnabled && bDisableTacticalFormationAIForTesting)
	{
		if (bForceFormationForTesting)
		{
			ApplyFormationDecision(BuildTestingFormationDecision());
		}
		return;
	}

	if (!TacticalAI)
		return;

	FGroupCombatContext Context = CreateCombatContext();

	FFormationDecision Decision = TacticalAI->CalculateDecision(Context);

	ApplyFormationDecision(Decision);
}

FGroupCombatContext UGroupCommandComponent::CreateCombatContext()
{
	FGroupCombatContext Context;

	Context.GroupSize = GroupMembers.Num();
	const FGroupTargetSelection TargetSelection = SelectFormationTarget(this, CurrentFormation, Leader, KnownEnemies);
	Context.Target = TargetSelection.TargetActor;
	Context.bTargetIsEnemyGroup = TargetSelection.bTargetIsEnemyGroup;
	Context.TargetGroupSize = TargetSelection.TargetGroupSize;
	Context.TargetGroupCenter = TargetSelection.TargetGroupCenter;

	// Use formation anchor location for distance to target when available;
	// fallback to leader location if no formation is set or anchor isn't valid.
	FVector AnchorPos = FVector::ZeroVector;
	if (CurrentFormation && CurrentFormation->HasValidAnchor())
	{
		AnchorPos = CurrentFormation->GetAnchorLocation();
	}
	else if (Leader && IsValid(Leader))
	{
		AnchorPos = Leader->GetActorLocation();
	}

	if (TargetSelection.TargetActor && AnchorPos != FVector::ZeroVector)
	{
		Context.DistanceToTarget = FVector::Dist(AnchorPos, TargetSelection.TargetLocation);
	}
	else
	{
		Context.DistanceToTarget = 0.f;
	}

	// Populate health and kinematics info from current group and target
	float SumNormHealth = 0.f;
	float MinNormHealth = 1.f;
	int32 ValidCount = 0;

	FVector Centroid = FVector::ZeroVector;
	TArray<AEnemyCore *> ActiveMembers;
	for (AEnemyCore *M : GroupMembers)
	{
		if (!IsValid(M))
			continue;
		ActiveMembers.Add(M);
		Centroid += M->GetActorLocation();
	}

	const int32 ActiveCount = ActiveMembers.Num();
	if (ActiveCount > 0)
	{
		Centroid /= ActiveCount;
	}

	float SumDistToCentroid = 0.f;
	int32 LowHealthCount = 0;
	for (AEnemyCore *M : ActiveMembers)
	{
		UHealthComponent *HC = M->GetHealthComponent();
		float Norm = 1.f;
		if (HC)
		{
			int Curr = HC->GetCurrentHealth();
			int Max = HC->GetMaxHealth();
			if (Max > 0)
			{
				Norm = FMath::Clamp((float)Curr / (float)Max, 0.f, 1.f);
			}
		}

		SumNormHealth += Norm;
		MinNormHealth = FMath::Min(MinNormHealth, Norm);
		ValidCount++;

		if (Norm <= 0.25f)
			LowHealthCount++;

		SumDistToCentroid += FVector::Dist(M->GetActorLocation(), Centroid);
	}

	if (ValidCount > 0)
	{
		Context.AverageGroupHealth = SumNormHealth / ValidCount;
		Context.LowestMemberHealth = MinNormHealth;
		Context.AverageDistanceBetweenMembers = SumDistToCentroid / ValidCount;
	}
	else
	{
		Context.AverageGroupHealth = 1.f;
		Context.LowestMemberHealth = 1.f;
		Context.AverageDistanceBetweenMembers = 0.f;
	}

	// Target info
	AActor *Target = TargetSelection.TargetActor;
	if (Target)
	{
		// try to get health component on target
		if (UHealthComponent *THC = Target->FindComponentByClass<UHealthComponent>())
		{
			Context.TargetHealth = THC->GetCurrentHealth();
		}

		Context.TargetVelocity = Target->GetVelocity().Size();
	}

	// Tactical booleans
	// bTargetInsideFormation: if formation is a circle use its radius; otherwise compare to member spread
	bool bInside = false;
	if (Target && AnchorPos != FVector::ZeroVector)
	{
		float D = FVector::Dist(AnchorPos, TargetSelection.TargetLocation);
		if (UEnemyFormation_Circle *Circle = Cast<UEnemyFormation_Circle>(CurrentFormation))
		{
			bInside = D <= Circle->Radius * 0.8f;
		}
		else
		{
			// compare to average member radius
			float AvgMemberRadius = Context.AverageDistanceBetweenMembers;
			bInside = (AvgMemberRadius > KINDA_SMALL_NUMBER) ? (D <= AvgMemberRadius * 0.8f) : false;
		}
	}
	Context.bTargetInsideFormation = bInside;

	// bGroupSpreadTooFar
	Context.bGroupSpreadTooFar = Context.AverageDistanceBetweenMembers > 400.f;

	// bTargetEscaping: target moving away from anchor
	bool bEscaping = false;
	if (Target && AnchorPos != FVector::ZeroVector)
	{
		FVector ToTarget = (TargetSelection.TargetLocation - AnchorPos).GetSafeNormal();
		FVector Vel = Target->GetVelocity();
		float Projection = FVector::DotProduct(Vel, ToTarget);
		bEscaping = Projection > 200.f; // moving away fast
	}
	Context.bTargetEscaping = bEscaping;

	// bTakingHeavyLosses: many members low on health
	Context.bTakingHeavyLosses = (ValidCount > 0) ? ((float)LowHealthCount / (float)ValidCount > 0.3f) : false;

	return Context;
}

void UGroupCommandComponent::ApplyFormationDecision(FFormationDecision Decision)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	CurrentBaseFormationDecision = Decision;
	bHasCurrentFormationDecision = true;

	TArray<AEnemyCore *> ActiveMembers;
	ActiveMembers.Reserve(GroupMembers.Num());
	for (AEnemyCore *M : GroupMembers)
	{
		if (IsValid(M))
			ActiveMembers.Add(M);
	}

	const FGroupTargetSelection TargetSelection = SelectFormationTarget(this, CurrentFormation, Leader, KnownEnemies);
	AActor *Target = TargetSelection.TargetActor;
	if (!Target)
		return;

	const FFormationDecision AppliedDecision = BuildEnvironmentAdaptedDecision(
		Decision,
		ActiveMembers,
		TargetSelection.TargetLocation,
		true);

	ApplyFormationParameters(AppliedDecision);

	// Reset formation anchor so new parameters are applied fresh
	if (CurrentFormation)
	{
		// Record applied formation type for learning labels
		CurrentAppliedFormation = AppliedDecision.FormationType;
		CurrentAppliedTuningPreset = AppliedDecision.TuningPreset;
		CurrentFormation->ResetFormation();

		const int32 Total = ActiveMembers.Num();

		// Ensure formation anchor is initialized for position generation
		// Provide leader context to formation and update anchor
		CurrentFormation->SetTargetLocationOverride(TargetSelection.TargetLocation);
		CurrentFormation->SetLeader(Leader);
		CurrentFormation->UpdateFormationAnchor(Leader, Target, ActiveMembers);

		// Generate desired positions for each slot
		TArray<FVector> DesiredPositions;
		DesiredPositions.SetNumZeroed(Total);
		for (int32 i = 0; i < Total; ++i)
		{
			DesiredPositions[i] = CurrentFormation->GetLocationForMember(ActiveMembers[i], i, Total, Target);
		}

		ClampFormationSlotsToEnvironmentWidth(DesiredPositions);

		TArray<EFormationMemberRole> EffectiveRoles;
		EffectiveRoles.SetNum(Total);
		for (int32 i = 0; i < Total; ++i)
		{
			EffectiveRoles[i] = GetPreferredRoleForMember(ActiveMembers[i], Leader);
		}

		// Assign positions to members minimizing total distance using the Hungarian algorithm
		if (Total > 0)
		{
			// We may need to recompute once if circle ends up with too many backline members
			int Attempts = 0;
			const int MaxAttempts = 2;
			TArray<int32> MemberAssigned;
			while (Attempts < MaxAttempts)
			{
				const FVector Anchor = CurrentFormation->GetAnchorLocation();
				const FVector Fwd = CurrentFormation->GetAnchorForward().GetSafeNormal();
				const FVector Right = FVector::CrossProduct(FVector::UpVector, Fwd).GetSafeNormal();
				const bool bIsCircleFormation = Cast<UEnemyFormation_Circle>(CurrentFormation) != nullptr;
				float MinForward = TNumericLimits<float>::Max();
				float MaxForward = -TNumericLimits<float>::Max();
				float MaxAbsLateral = 0.f;
				float MaxRadius = 0.f;
				for (const FVector &SlotLocation : DesiredPositions)
				{
					const FVector Offset = SlotLocation - Anchor;
					const float ForwardValue = FVector::DotProduct(Offset, Fwd);
					const float AbsLateral = FMath::Abs(FVector::DotProduct(Offset, Right));
					MinForward = FMath::Min(MinForward, ForwardValue);
					MaxForward = FMath::Max(MaxForward, ForwardValue);
					MaxAbsLateral = FMath::Max(MaxAbsLateral, AbsLateral);
					MaxRadius = FMath::Max(MaxRadius, Offset.Size2D());
				}

				// Build cost matrix (members x positions)
				TArray<TArray<float>> Cost;
				Cost.SetNum(Total);
				for (int32 i = 0; i < Total; ++i)
				{
					Cost[i].SetNum(Total);
					for (int32 j = 0; j < Total; ++j)
					{
						Cost[i][j] = FVector::Dist(ActiveMembers[i]->GetActorLocation(), DesiredPositions[j]);
						if (ActiveMembers[i] == Leader && ActiveMembers[j] != Leader)
						{
							Cost[i][j] += 100000.f;
						}
						else if (ActiveMembers[i] != Leader && ActiveMembers[j] == Leader)
						{
							Cost[i][j] += 100000.f;
						}

						Cost[i][j] += GetRoleSlotCost(
							EffectiveRoles[i],
							DesiredPositions[j],
							Anchor,
							Fwd,
							Right,
							MinForward,
							MaxForward,
							MaxAbsLateral,
							MaxRadius,
							bIsCircleFormation);
					}
				}

				// Hungarian algorithm (1-based indexing)
				const float INF = TNumericLimits<float>::Max() / 4.f;
				TArray<float> u;
				u.Init(0.f, Total + 1);
				TArray<float> v;
				v.Init(0.f, Total + 1);
				TArray<int32> p;
				p.Init(0, Total + 1);
				TArray<int32> way;
				way.Init(0, Total + 1);

				for (int32 i = 1; i <= Total; ++i)
				{
					p[0] = i;
					int32 j0 = 0;
					TArray<float> minv;
					minv.Init(INF, Total + 1);
					TArray<char> used;
					used.Init(0, Total + 1);
					do
					{
						used[j0] = 1;
						int32 i0 = p[j0];
						int32 j1 = 0;
						float delta = INF;
						for (int32 j = 1; j <= Total; ++j)
						{
							if (used[j])
								continue;
							float cur = Cost[i0 - 1][j - 1] - u[i0] - v[j];
							if (cur < minv[j])
							{
								minv[j] = cur;
								way[j] = j0;
							}
							if (minv[j] < delta)
							{
								delta = minv[j];
								j1 = j;
							}
						}
						for (int32 j = 0; j <= Total; ++j)
						{
							if (used[j])
							{
								u[p[j]] += delta;
								v[j] -= delta;
							}
							else
							{
								minv[j] -= delta;
							}
						}
						j0 = j1;
					} while (p[j0] != 0);

					do
					{
						int32 j1 = way[j0];
						p[j0] = p[j1];
						j0 = j1;
					} while (j0 != 0);
				}

				// p[j] = assigned row for column j
				MemberAssigned.Init(-1, Total);
				for (int32 j = 1; j <= Total; ++j)
				{
					if (p[j] > 0)
					{
						MemberAssigned[p[j] - 1] = j - 1;
					}
				}

				const int32 LeaderMemberIndex = ActiveMembers.IndexOfByKey(Leader);
				if (LeaderMemberIndex != INDEX_NONE && MemberAssigned.IsValidIndex(LeaderMemberIndex))
				{
					const int32 LeaderSlotIndex = LeaderMemberIndex;
					const int32 CurrentLeaderSlot = MemberAssigned[LeaderMemberIndex];

					if (CurrentLeaderSlot != LeaderSlotIndex)
					{
						for (int32 m = 0; m < Total; ++m)
						{
							if (m != LeaderMemberIndex && MemberAssigned[m] == LeaderSlotIndex)
							{
								MemberAssigned[m] = CurrentLeaderSlot;
								break;
							}
						}

						MemberAssigned[LeaderMemberIndex] = LeaderSlotIndex;
					}
				}

				// Optionally perform backline swapping based on decision
				if (bEnableBacklineSwapping && AppliedDecision.bEnableBacklineSwap)
				{
					// Compute normalized HP for each active member
					TArray<float> NormHP;
					NormHP.Init(1.f, Total);
					for (int32 m = 0; m < Total; ++m)
					{
						AEnemyCore *M = ActiveMembers[m];
						if (!IsValid(M))
						{
							NormHP[m] = 0.f;
							continue;
						}
						if (UHealthComponent *HC = M->GetHealthComponent())
						{
							float curr = (float)HC->GetCurrentHealth();
							float maxh = (float)HC->GetMaxHealth();
							NormHP[m] = (maxh > KINDA_SMALL_NUMBER) ? FMath::Clamp(curr / maxh, 0.f, 1.f) : 1.f;
						}
					}

					// Classify assigned positions into front/back by dot with formation forward
					TArray<int32> FrontMembers;
					TArray<int32> BackMembers;
					for (int32 m = 0; m < Total; ++m)
					{
						if (ActiveMembers[m] == Leader)
							continue;

						int32 pos = MemberAssigned[m];
						if (pos < 0 || pos >= DesiredPositions.Num())
							continue;
						FVector SlotDir = (DesiredPositions[pos] - Anchor).GetSafeNormal();
						float dot = FVector::DotProduct(SlotDir, Fwd);
						if (dot > 0.f)
							FrontMembers.Add(m);
						else
							BackMembers.Add(m);
					}

					// Sort front ascending by hp, back descending by hp
					FrontMembers.Sort([&](int32 A, int32 B)
									  { return NormHP[A] < NormHP[B]; });
					BackMembers.Sort([&](int32 A, int32 B)
									 { return NormHP[A] > NormHP[B]; });

					float Threshold = FMath::Clamp(AppliedDecision.BacklineHpDiffThresholdPercent, 0.f, 100.f) / 100.f;

					TArray<bool> BackUsed;
					BackUsed.Init(false, BackMembers.Num());
					for (int32 fi = 0; fi < FrontMembers.Num(); ++fi)
					{
						int32 frontIdx = FrontMembers[fi];
						for (int32 bi = 0; bi < BackMembers.Num(); ++bi)
						{
							if (BackUsed[bi])
								continue;
							int32 backIdx = BackMembers[bi];
							if (NormHP[backIdx] - NormHP[frontIdx] >= Threshold)
							{
								// swap assigned positions
								int32 posF = MemberAssigned[frontIdx];
								int32 posB = MemberAssigned[backIdx];
								MemberAssigned[frontIdx] = posB;
								MemberAssigned[backIdx] = posF;
								EffectiveRoles[frontIdx] = EFormationMemberRole::Backline;
								EffectiveRoles[backIdx] = GetPreferredRoleForMember(ActiveMembers[backIdx], Leader);
								BackUsed[bi] = true;
								break;
							}
						}
					}

					// After swaps, count how many non-leader members remain in the front line
					int32 FrontCount = 0;
					for (int32 m = 0; m < Total; ++m)
					{
						if (ActiveMembers[m] == Leader)
							continue;

						int32 pos = MemberAssigned[m];
						if (pos < 0 || pos >= DesiredPositions.Num())
							continue;
						FVector SlotDir = (DesiredPositions[pos] - Anchor).GetSafeNormal();
						float dot = FVector::DotProduct(SlotDir, Fwd);
						if (dot > 0.f)
							FrontCount++;
					}

					// If a compressed circle arc has too few non-leader front-line members,
					// switch to line and retry once. Full 360-degree circles are preserved.
					const bool bPreserveFullCircle =
						AppliedDecision.FormationType == EFormationType::Circle &&
						AppliedDecision.CircleArcAngleDegrees >= 300.f;
					if (Cast<UEnemyFormation_Circle>(CurrentFormation) && !bPreserveFullCircle && FrontCount < 4 && Attempts == 0)
					{
						// Switch formation
						CurrentFormation = NewObject<UEnemyFormation_Line>(this);
						CurrentAppliedFormation = EFormationType::Line;
						CurrentAppliedTuningPreset = EFormationTuningPreset::Balanced;

						// apply decision line params if present
						if (AppliedDecision.bSetLineParams)
						{
							UEnemyFormation_Line *LF = Cast<UEnemyFormation_Line>(CurrentFormation);
							if (LF)
							{
								LF->MinSpacing = AppliedDecision.LineMinSpacing;
								LF->MaxLineLength = AppliedDecision.LineMaxLineLength;
								LF->RowOffset = AppliedDecision.LineRowOffset;
							}
						}
						// rebuild desired positions for new formation
						CurrentFormation->ResetFormation();
						CurrentFormation->SetTargetLocationOverride(TargetSelection.TargetLocation);
						CurrentFormation->SetLeader(Leader);
						CurrentFormation->UpdateFormationAnchor(Leader, Target, ActiveMembers);
						for (int32 i = 0; i < Total; ++i)
						{
							DesiredPositions[i] = CurrentFormation->GetLocationForMember(ActiveMembers[i], i, Total, Target);
						}
						ClampFormationSlotsToEnvironmentWidth(DesiredPositions);
						Attempts++;
						continue; // retry Hungarian with new formation
					}
				}

				// If no early retry required, accept MemberAssigned and break
				// Build new ordered members array based on assigned positions
				TArray<AEnemyCore *> NewGroupMembers;
				NewGroupMembers.SetNumZeroed(Total);
				for (int32 m = 0; m < Total; ++m)
				{
					int32 Pos = MemberAssigned[m];
					if (Pos >= 0 && Pos < Total)
					{
						NewGroupMembers[Pos] = ActiveMembers[m];
					}
				}

				GroupMembers = NewGroupMembers;
				break;
			} // attempts
		}

		// Replace GroupMembers with new ordering (only valid members)
		// (already assigned above for the Hungarian result)

		// Rebuild mapping and immediately command controllers to go to assigned positions
		RebuildFormationIndices();
		TArray<FVector> CorrectedPositions = DesiredPositions;
		TArray<EFormationMemberRole> OrderedRoles;
		OrderedRoles.SetNum(Total);
		for (int32 p = 0; p < Total; ++p)
		{
			AEnemyCore *Member = GroupMembers[p];
			EFormationMemberRole Role = GetPreferredRoleForMember(Member, Leader);
			const int32 OriginalMemberIndex = ActiveMembers.IndexOfByKey(Member);
			if (EffectiveRoles.IsValidIndex(OriginalMemberIndex))
			{
				Role = EffectiveRoles[OriginalMemberIndex];
			}
			OrderedRoles[p] = Role;
		}

		ApplyPositionBasedSlotCorrection(GroupMembers, OrderedRoles, DesiredPositions, CorrectedPositions);

		UpdateFormationAttackRelease(GroupMembers, CorrectedPositions, TargetSelection.TargetLocation);

		for (int32 p = 0; p < Total; ++p)
		{
			AEnemyCore *Member = GroupMembers[p];
			if (!IsValid(Member))
				continue;

			const EFormationMemberRole Role = OrderedRoles.IsValidIndex(p) ? OrderedRoles[p] : GetPreferredRoleForMember(Member, Leader);

			Member->SetCurrentFormationRole(Role);

			ACoreAIController *Controller = Cast<ACoreAIController>(Member->GetController());
			if (!Controller)
				continue;

			Controller->SetFormationLocation(CorrectedPositions[p]);
			Controller->SetFormationRole(Role);
		}
	}
}
