#include "EnemySpawner.h"
#include "multiplayerTEST/Characters/Enemies/EnemyCore.h"
#include "multiplayerTEST/Subsystems/WorldSubsystems/RegistrySubsystem.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/SaveGameSubsystem.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "Engine/World.h"
#include "multiplayerTEST/Components/EnemyComponents/GroupCommandComponent.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"

AEnemySpawner::AEnemySpawner()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	RootComponent = MeshComp;

	SetActorHiddenInGame(true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SpawnedEnemy = nullptr;
	isOneTimeEnemy = false;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnerID.IsNone())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("EnemySpawner %s has empty SpawnerID - will NOT spawn enemy"),
			*GetName());
		return;
	}
	Registry = GetWorld()->GetSubsystem<URegistrySubsystem>();
	if (Registry)
	{
		Registry->RegisterSpawner(this);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		SaveSubsystem = GI->GetSubsystem<USaveGameSubsystem>();
	}

	if (HasAuthority())
	{
		SpawnEnemy();
	}
}

void AEnemySpawner::SpawnEnemy()
{
	if (!EnemyClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemySpawner: EnemyClass is null"));
		return;
	}

	if (SpawnedEnemy)
		return;

	if (SaveSubsystem && SaveSubsystem->IsEnemyDead(SpawnerID, isOneTimeEnemy))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
		return;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	SpawnedEnemy = World->SpawnActor<AEnemyCore>(EnemyClass, GetActorTransform(), Params);

	if (!SpawnedEnemy) return;

	if (UGameplayTagManagerComp* TagManager = SpawnedEnemy->FindComponentByClass<UGameplayTagManagerComp>())
	{
		if (bOverrideOwnerFactionTag)
		{
			TagManager->SetOwnerFactionTag(OwnerFactionTagOverride);
		}

		if (bOverrideOwnerEnemiesFactionTags)
		{
			TagManager->SetOwnerEnemiesFactionTags(OwnerEnemiesFactionTagsOverride);
		}
	}

	if (bOverrideFormationRole)
	{
		SpawnedEnemy->SetPreferredFormationRole(FormationRoleOverride);
	}

	SpawnedEnemy->SpawnDefaultController();

	if (UHealthComponent* HC = SpawnedEnemy->FindComponentByClass<UHealthComponent>())
	{
		HC->ActorDied.AddDynamic(this, &AEnemySpawner::OnEnemyDied);
	}

	if (UGroupCommandComponent* CommandComp = SpawnedEnemy->FindComponentByClass<UGroupCommandComponent>()) {
		CommandComp->SetUpComponent(SpawnedEnemy, EnemiesToCommand);

		const bool bApplyLeaderTesting =
			bOverrideFormationRole &&
			FormationRoleOverride == EFormationMemberRole::Leader &&
			bEnableLeaderFormationTestSettings;

		CommandComp->SetFormationTestingOverrides(
			bApplyLeaderTesting,
			bDisableTacticalFormationAIForTesting,
			bForceFormationForTesting,
			ForcedFormationForTesting,
			ForcedFormationTuningPreset,
			bSuppressTacticalLearningWhileTesting);
	}

	if (Registry)
	{
		Registry->OnEnemySpawned.Broadcast(SpawnedEnemy, SpawnerID);
	}
}


void AEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Registry)
	{
		Registry->UnregisterSpawner(this);
	}

	if (HasAuthority() && SpawnedEnemy && !SpawnedEnemy->IsPendingKillPending())
	{
		SpawnedEnemy->Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

void AEnemySpawner::OnEnemyDied(AActor* DiedActor)
{
	if (SaveSubsystem)
	{
		SaveSubsystem->MarkEnemyDead(SpawnerID, isOneTimeEnemy);
	}
}

void AEnemySpawner::ResetSpawner()
{
	if (!HasAuthority())
		return;

	if (SaveSubsystem && SaveSubsystem->IsEnemyDead(SpawnerID, isOneTimeEnemy))
	{
		return;
	}

	if (!SpawnedEnemy) {
		SpawnEnemy();
		return;
	}

	if (!SpawnedEnemy->bNeedReset) return;

	SpawnedEnemy->Destroy();
	SpawnedEnemy = nullptr;
	SpawnEnemy();

}
