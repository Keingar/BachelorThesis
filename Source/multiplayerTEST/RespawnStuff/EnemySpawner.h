// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/FormationDecision.h"
#include "EnemySpawner.generated.h"

class AEnemyCore;
class UStaticMeshComponent;
class USaveGameSubsystem;
class URegistrySubsystem;

UCLASS()
class MULTIPLAYERTEST_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent *MeshComp;

public:
	AEnemySpawner();

	FName GetSpawnerID() const { return SpawnerID; }
	AEnemyCore *GetSpawnedEnemy() const { return SpawnedEnemy; }
	void ResetSpawner();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly)
	TSubclassOf<AEnemyCore> EnemyClass;

	UPROPERTY(EditInstanceOnly)
	bool isOneTimeEnemy;

	UPROPERTY(EditInstanceOnly)
	TArray<FName> EnemiesToCommand;

	UPROPERTY(EditInstanceOnly)
	bool bOverrideFormationRole = false;

	UPROPERTY(EditInstanceOnly, meta = (EditCondition = "bOverrideFormationRole"))
	EFormationMemberRole FormationRoleOverride = EFormationMemberRole::Defender;

	UPROPERTY(EditInstanceOnly, meta = (EditCondition = "bOverrideFormationRole && FormationRoleOverride == EFormationMemberRole::Leader", EditConditionHides))
	bool bEnableLeaderFormationTestSettings = false;

	UPROPERTY(EditInstanceOnly, meta = (EditCondition = "bOverrideFormationRole && FormationRoleOverride == EFormationMemberRole::Leader && bEnableLeaderFormationTestSettings", EditConditionHides))
	bool bDisableTacticalFormationAIForTesting = false;

	UPROPERTY(EditInstanceOnly, meta = (EditCondition = "bOverrideFormationRole && FormationRoleOverride == EFormationMemberRole::Leader && bEnableLeaderFormationTestSettings && bDisableTacticalFormationAIForTesting", EditConditionHides))
	bool bForceFormationForTesting = false;

	UPROPERTY(EditInstanceOnly, meta = (EditCondition = "bOverrideFormationRole && FormationRoleOverride == EFormationMemberRole::Leader && bEnableLeaderFormationTestSettings && bDisableTacticalFormationAIForTesting && bForceFormationForTesting", EditConditionHides))
	EFormationType ForcedFormationForTesting = EFormationType::Circle;

	UPROPERTY(EditInstanceOnly, meta = (EditCondition = "bOverrideFormationRole && FormationRoleOverride == EFormationMemberRole::Leader && bEnableLeaderFormationTestSettings && bDisableTacticalFormationAIForTesting && bForceFormationForTesting", EditConditionHides))
	EFormationTuningPreset ForcedFormationTuningPreset = EFormationTuningPreset::Balanced;

	UPROPERTY(EditInstanceOnly, meta = (EditCondition = "bOverrideFormationRole && FormationRoleOverride == EFormationMemberRole::Leader && bEnableLeaderFormationTestSettings", EditConditionHides))
	bool bSuppressTacticalLearningWhileTesting = true;

	UPROPERTY(EditInstanceOnly)
	bool bOverrideOwnerFactionTag = false;

	UPROPERTY(EditInstanceOnly, meta = (EditCondition = "bOverrideOwnerFactionTag"))
	FGameplayTag OwnerFactionTagOverride;

	UPROPERTY(EditInstanceOnly)
	bool bOverrideOwnerEnemiesFactionTags = false;

	UPROPERTY(EditInstanceOnly, meta = (EditCondition = "bOverrideOwnerEnemiesFactionTags"))
	FGameplayTagContainer OwnerEnemiesFactionTagsOverride;

	UPROPERTY()
	AEnemyCore *SpawnedEnemy;

	void SpawnEnemy();
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditInstanceOnly)
	FName SpawnerID;

	UFUNCTION()
	void OnEnemyDied(AActor *DiedActor);

private:
	UPROPERTY()
	USaveGameSubsystem *SaveSubsystem = nullptr;
	UPROPERTY()
	URegistrySubsystem *Registry = nullptr;
};
