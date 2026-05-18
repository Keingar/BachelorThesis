#include "RegistrySubsystem.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/SaveGameSubsystem.h"
#include "multiplayerTEST/RespawnStuff/EnemySpawner.h"
#include "multiplayerTEST/Characters/Enemies/EnemyCore.h"
#include "multiplayerTEST/GameplayInterfaces/Resettable.h"

void URegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			SaveSubsystem = GI->GetSubsystem<USaveGameSubsystem>();
		}
	}
}

void URegistrySubsystem::RegisterSpawner(AEnemySpawner* Spawner)
{
	if (!Spawner)
		return;

	FName ID = Spawner->GetSpawnerID();

	if (ID.IsNone())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Spawner %s tried to register with EMPTY ID"),
			*Spawner->GetName());
		return;
	}

	// Duplicate detection
	if (RegisteredSpawners.Contains(ID))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Duplicate SpawnerID detected: %s"),
			*ID.ToString());
	}

	RegisteredSpawners.Add(ID, Spawner);
	OnSpawnerRegistered.Broadcast(Spawner);
}

void URegistrySubsystem::UnregisterSpawner(AEnemySpawner* Spawner)
{
	if (!Spawner)
		return;

	FName ID = Spawner->GetSpawnerID();

	if (ID.IsNone())
		return;

	RegisteredSpawners.Remove(ID);
}

AEnemySpawner* URegistrySubsystem::GetSpawnerByID(FName SpawnerID) const
{
	if (const TWeakObjectPtr<AEnemySpawner>* Found =
		RegisteredSpawners.Find(SpawnerID))
	{
		return Found->IsValid() ? Found->Get() : nullptr;
	}

	return nullptr;
}

AEnemyCore* URegistrySubsystem::GetEnemyByID(FName SpawnerID) const
{
	AEnemySpawner* Spawner = GetSpawnerByID(SpawnerID);

	if (!Spawner)
		return nullptr;

	return Spawner->GetSpawnedEnemy();
}

void URegistrySubsystem::RegisterObject(FName ObjectID, AActor* ObjectActor)
{
	if (ObjectID.IsNone() || !ObjectActor)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Tried to register object with invalid ID or Actor"));
		return;
	}

	if (RegisteredObjects.Contains(ObjectID))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Duplicate ObjectID detected: %s"),
			*ObjectID.ToString());
	}

	RegisteredObjects.Add(ObjectID, ObjectActor);
}

void URegistrySubsystem::UnregisterObject(FName ObjectID)
{
	if (ObjectID.IsNone())
		return;

	RegisteredObjects.Remove(ObjectID);
}

AActor* URegistrySubsystem::GetObjectByID(FName ObjectID) const
{
	if (const TWeakObjectPtr<AActor>* Found =
		RegisteredObjects.Find(ObjectID))
	{
		return Found->IsValid() ? Found->Get() : nullptr;
	}

	return nullptr;
}

bool URegistrySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	if (!World)
	{
		return false;
	}

	// Do not create in editor preview worlds
	if (World->WorldType != EWorldType::Game &&
		World->WorldType != EWorldType::PIE)
	{
		return false;
	}

	// Block specific levels (start menu, front end, etc.)
	const FString MapName = World->GetMapName();

	if (MapName.Contains(TEXT("StartMenu")))
	{
		return false;
	}

	return true;
}

void URegistrySubsystem::ResetObjects()
{
	for (auto& Pair : RegisteredObjects)
	{
		AActor* Actor = Pair.Value.Get();
		if (!Actor) continue;

		if (Actor->GetClass()->ImplementsInterface(UResettable::StaticClass()))
		{
			IResettable::Execute_ResetObject(Actor);
		}
	}
}

void URegistrySubsystem::ResetAllSpawners()
{
	for (auto& Pair : RegisteredSpawners)
	{
		if (AEnemySpawner* Spawner = Pair.Value.Get())
		{
			Spawner->ResetSpawner();
		}
	}
}

void URegistrySubsystem::ResetObjectsAndSpawners()
{
	if (GetWorld()->IsNetMode(NM_Client) || !SaveSubsystem) return;

	bResetEnded = false;
	SaveSubsystem->ClearDeadEnemies();

	ResetObjects();
	ResetAllSpawners();
		
	bResetEnded = true;
	OnLevelResetEnded.Broadcast();

	SaveSubsystem->ForceSave();
}