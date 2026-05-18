#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "multiplayerTEST/AI/Formations/FormationEnvironmentAnalyzer.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/FormationDecision.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/GroupCombatContext.h"
#include "GroupCommandComponent.generated.h"

class AEnemyCore;
class AEnemySpawner;
class URegistrySubsystem;
class UEnemyFormation;
class UEnemyGroupTacticalAI;
enum class ECombatOutcome : uint8;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIPLAYERTEST_API UGroupCommandComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UFUNCTION()
	void OnMemberDiedOrDestroyed(AActor* Member);

    // (Removed dynamic fight/health handlers - sampling approach used instead)

	UFUNCTION()
	void OnKnownEnemyDied(AActor* DeadEnemy);

	UFUNCTION()
	void OnRegistryEnemySpawned(AEnemyCore* Enemy, FName SpawnerID);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FVector GetFormationLocation(AEnemyCore* Member);

	FTimerHandle FormationUpdateTimer;
	FTimerHandle AIDecisionUpdateTimer;

	void UpdateFormation();

	void RebuildFormationIndices();
	void ApplyMinimalTravelReindexing(TArray<AEnemyCore*>& InOutOrderedMembers, const TArray<FVector>& SlotPositions) const;
	void UpdateFormationAttackRelease(
		const TArray<AEnemyCore*>& OrderedMembers,
		const TArray<FVector>& SlotPositions,
		const FVector& TargetLocation);
	void ResetFormationAttackRelease();
	void SetMemberShouldAttackInFormation(AEnemyCore* Member, bool bShouldAttack) const;

	void UpdateTacticalDecision();
	FGroupCombatContext CreateCombatContext();

	void ApplyFormationDecision(FFormationDecision Decision);
	FFormationDecision BuildTestingFormationDecision() const;
	void ApplyFormationParameters(const FFormationDecision& Decision);
	FFormationDecision BuildEnvironmentAdaptedDecision(
		const FFormationDecision& BaseDecision,
		const TArray<AEnemyCore*>& ActiveMembers,
		const FVector& TargetLocation,
		bool bForceRefresh);
	FFormationDecision AdaptFormationDecisionForEnvironment(
		const FFormationDecision& BaseDecision,
		const FFormationEnvironmentAnalysisResult& Environment,
		int32 ActiveMemberCount) const;
	float EstimateDesiredFormationWidth(const FFormationDecision& Decision, int32 ActiveMemberCount) const;
	void LogEnvironmentAdaptationIfNeeded(const FFormationEnvironmentAnalysisResult& Environment);
	void ClampFormationSlotsToEnvironmentWidth(TArray<FVector>& InOutPositions) const;
	void SubmitCombatLearningResult(ECombatOutcome Outcome, float DamageDealt, float DamageTaken);
	void ApplyPositionBasedSlotCorrection(
		const TArray<AEnemyCore*>& OrderedMembers,
		const TArray<EFormationMemberRole>& OrderedRoles,
		const TArray<FVector>& OriginalPositions,
		TArray<FVector>& InOutPositions
	) const;

	TArray<FName> DefaultMemberIDs;

	//the ones that failed to add at first begin play
	TArray<FName> LateDefaultMembersToAdd;

	URegistrySubsystem* Registry;

    UPROPERTY()
	UEnemyFormation* CurrentFormation;

	// Tactical AI owned by the component (UPROPERTY to keep it alive and visible to GC)
	UPROPERTY()
	UEnemyGroupTacticalAI* TacticalAI;

	UPROPERTY()
	FFormationDecision CurrentBaseFormationDecision;

	UPROPERTY()
	FFormationDecision CurrentEnvironmentAdaptedDecision;

	bool bHasCurrentFormationDecision = false;
	FFormationEnvironmentAnalysisResult CachedEnvironmentAnalysis;
	double LastEnvironmentAnalysisTime = -100000.0;
	FVector LastEnvironmentAnalysisOrigin = FVector::ZeroVector;
	FVector LastEnvironmentAnalysisForward = FVector::ForwardVector;
	EFormationEnvironmentMode LastLoggedEnvironmentMode = EFormationEnvironmentMode::Open;
	bool bHasLoggedEnvironmentMode = false;

	bool bLeaderFormationTestOverridesEnabled = false;
	bool bDisableTacticalFormationAIForTesting = false;
	bool bForceFormationForTesting = false;
	EFormationType ForcedFormationForTesting = EFormationType::Circle;
	EFormationTuningPreset ForcedFormationTuningPreset = EFormationTuningPreset::Balanced;
	bool bSuppressTacticalLearningWhileTesting = true;

	TMap<AEnemyCore*, double> FormationAttackReleaseExpiry;
	double LastFormationAttackReleaseSelectionTime = -100000.0;
public:
    UPROPERTY()
	TArray<AEnemyCore*> GroupMembers;

	UPROPERTY()
	TMap<AEnemyCore*, int32> MemberFormationIndex;

	AEnemyCore* Leader;

	bool bGroupInCombat = false;
	bool bCombatLearningSubmitted = false;

    UPROPERTY()
	TArray<AActor*> KnownEnemies;

	// Currently applied formation type (used for learning labels)
	UPROPERTY()
	EFormationType CurrentAppliedFormation = EFormationType::None;

	UPROPERTY()
	EFormationTuningPreset CurrentAppliedTuningPreset = EFormationTuningPreset::Balanced;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction")
	bool bEnablePositionBasedSlotCorrection = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Compatibility")
	bool bUseClassicFormationPathing = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Assignment")
	bool bUseMinimalTravelReindexing = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Assignment")
	bool bEnableBacklineSwapping = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Attack Release")
	bool bEnableFormationAttackRelease = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Attack Release", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float FormationAttackReleasePercent = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Attack Release", meta = (ClampMin = "1", UIMin = "1", UIMax = "12"))
	int32 FormationAttackReleaseMaxCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Attack Release", meta = (ClampMin = "1", UIMin = "1", UIMax = "20"))
	int32 FormationAttackReleaseMinNonLeaderCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Attack Release", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1000.0"))
	float FormationAttackReadyDistance = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Attack Release", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float FormationAttackReadyMemberFraction = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Attack Release", meta = (ClampMin = "1.0", UIMin = "1.0", UIMax = "60.0"))
	float FormationAttackHoldSeconds = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Attack Release", meta = (ClampMin = "0.5", UIMin = "0.5", UIMax = "15.0"))
	float FormationAttackRefreshSeconds = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction", meta = (ClampMin = "0", UIMin = "0", UIMax = "8"))
	int32 SlotCorrectionIterations = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SlotCorrectionMinDistance = 140.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float SlotCorrectionStrength = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SlotCorrectionMaxOffset = 220.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction")
	bool bProjectCorrectedSlotsToNavMesh = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction")
	bool bDrawSlotCorrectionDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction|PBMAD")
	bool bEnablePredictiveSlotAvoidance = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction|PBMAD", meta = (ClampMin = "0.05", UIMin = "0.05", UIMax = "1.5"))
	float SlotCorrectionPredictionTime = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction|PBMAD", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float SlotCorrectionPreferredVelocityBlend = 0.65f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction|PBMAD", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "3.0"))
	float SlotCorrectionRadiusScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction|PBMAD", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float SlotCorrectionShapePreservationStrength = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction|PBMAD")
	bool bEnableSlotCorrectionCohesion = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction|PBMAD", meta = (ClampMin = "0.0", ClampMax = "0.5", UIMin = "0.0", UIMax = "0.5"))
	float SlotCorrectionCohesionStrength = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction|PBMAD")
	bool bEnableRoleAwareSlotCorrection = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction|Debug")
	bool bDrawSlotCorrectionRadii = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Slot Correction|Debug")
	bool bDrawSlotCorrectionPredictionLinks = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment")
	bool bEnableEnvironmentAwareFormationAdaptation = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment")
	bool bClampFormationSlotsToEnvironmentWidth = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "5.0"))
	float EnvironmentAnalysisInterval = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment", meta = (ClampMin = "25.0", UIMin = "25.0", UIMax = "300.0"))
	float EnvironmentProbeStep = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment", meta = (ClampMin = "100.0", UIMin = "100.0", UIMax = "2500.0"))
	float EnvironmentMaxProbeDistance = 900.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment", meta = (ClampMin = "25.0", UIMin = "25.0", UIMax = "1000.0"))
	float EnvironmentAnalysisMoveThreshold = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment")
	FVector EnvironmentProbeProjectionExtent = FVector(50.f, 50.f, 250.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "300.0"))
	float EnvironmentProjectionTolerance = 45.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float LimitedWidthScaleThreshold = 0.85f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float NarrowWidthScaleThreshold = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment", meta = (ClampMin = "0.1", ClampMax = "1.0", UIMin = "0.1", UIMax = "1.0"))
	float EnvironmentLimitedCompressionMin = 0.55f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment", meta = (ClampMin = "0.1", ClampMax = "1.0", UIMin = "0.1", UIMax = "1.0"))
	float EnvironmentWidthSafetyMargin = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment", meta = (ClampMin = "100.0", UIMin = "100.0", UIMax = "1000.0"))
	float EnvironmentNarrowForceLineWidth = 350.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment", meta = (ClampMin = "2", UIMin = "2", UIMax = "50"))
	int32 EnvironmentLargeGroupForceLineCount = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment|Debug")
	bool bDrawEnvironmentAdaptationDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Formation|Environment|Debug")
	bool bLogEnvironmentAdaptation = false;

	// Combat tracking
	double CombatStartTime = 0.0;
	float AccumulatedDamageDealt = 0.f;
	float AccumulatedDamageTaken = 0.f;
	int32 ActiveFightersCount = 0;

	// Health snapshots for delta computations
	TMap<AActor*, float> LastKnownEnemyHealth;
	TMap<AEnemyCore*, float> LastMemberHealth;

	UFUNCTION()
	void OnMemberStartedFight(AActor* NewEnemy);
	void AlertGroup(AActor* EnemyActor);

	void AddMember(AEnemyCore* NewMember);

	void SetUpComponent(AEnemyCore* NewLeader, TArray<FName> DefaultEnemiesToCommand);

	void SetFormationTestingOverrides(
		bool bEnableOverrides,
		bool bDisableTacticalAI,
		bool bForceFormation,
		EFormationType ForcedFormation,
		EFormationTuningPreset ForcedPreset,
		bool bSuppressLearning);

	UFUNCTION()
	void SetUpDefaultSquad();
};
