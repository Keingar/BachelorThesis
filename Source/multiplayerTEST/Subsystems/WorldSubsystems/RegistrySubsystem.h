#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RegistrySubsystem.generated.h"

class AEnemySpawner;
class AEnemyCore;
class USaveGameSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelResetEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpawnerSpawned, AEnemySpawner*, Spawner);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemySpawned, AEnemyCore*, Enemy, FName, EnemySpawnerID);

UCLASS()
class MULTIPLAYERTEST_API URegistrySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ResetObjectsAndSpawners();

	// enemy spawners logic
public:
	bool bResetEnded = true;

	UPROPERTY(BlueprintAssignable)
	FOnLevelResetEnded OnLevelResetEnded;
	UPROPERTY(BlueprintAssignable)
	FOnSpawnerSpawned OnSpawnerRegistered;
	UPROPERTY(BlueprintAssignable)
	FOnEnemySpawned OnEnemySpawned;

	void RegisterSpawner(AEnemySpawner* Spawner);
	void UnregisterSpawner(AEnemySpawner* Spawner);

	AEnemySpawner* GetSpawnerByID(FName SpawnerID) const;

	UFUNCTION(BlueprintCallable)
	AEnemyCore* GetEnemyByID(FName SpawnerID) const;

	const TMap<FName, TWeakObjectPtr<AEnemySpawner>>& GetAllSpawners() const
	{
		return RegisteredSpawners;
	}

	// interactable objects logic
public:

	void RegisterObject(FName ObjectID, AActor* ObjectActor);
	void UnregisterObject(FName ObjectID);

	UFUNCTION(BlueprintCallable)
	AActor* GetObjectByID(FName ObjectID) const;

	const TMap<FName, TWeakObjectPtr<AActor>>& GetAllObjects() const
	{
		return RegisteredObjects;
	}

protected:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	void ResetObjects();
	void ResetAllSpawners();

	UPROPERTY()
	TMap<FName, TWeakObjectPtr<AEnemySpawner>> RegisteredSpawners;

	UPROPERTY()
	TMap<FName, TWeakObjectPtr<AActor>> RegisteredObjects;

	UPROPERTY()
	USaveGameSubsystem* SaveSubsystem = nullptr;
};
