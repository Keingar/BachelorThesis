#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "multiplayerTEST/GameplayInterfaces/Interactables/SpawnOverlapHandleInterface.h"
#include "Ship_Visual.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UMyBuoyancyComponent;
class AShip_Ghost;
class AShipPlayerPassengerGhost;
class AShipPassangerGhostCore;
class AShipEnemyPassangerGhost;

UCLASS()
class MULTIPLAYERTEST_API AShip_Visual : public AActor, public ISpawnOverlapHandleInterface
{
	GENERATED_BODY()

public:
	AShip_Visual();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* ShipMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* BoardingVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UMyBuoyancyComponent* BuoyancyComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AShip_Ghost* GhostShip;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AShipPlayerPassengerGhost> PlayerPassengerGhostClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AShipEnemyPassangerGhost> EnemyPassengerGhostClass;

	UFUNCTION(BlueprintCallable)
	void BoardPassenger(ACharacter* RealCharacter);

	UFUNCTION(BlueprintCallable)
	void UnboardPassenger(ACharacter* RealCharacter);

	void BeginOverlapAtSpawn_Implementation(AActor* OverlappedActor) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	void SyncGhostShipTransform();

	void SetUpGhostMeshAndAnimInstance(AShipPassangerGhostCore* Ghost, ACharacter* RealChar);

	UFUNCTION()
	void OnBoardingVolumeBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnBoardingVolumeEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);


	void SetUpGhostAnimInstanceSync(AShipPassangerGhostCore* Ghost, ACharacter* RealCharacter);

	UPROPERTY()
	TMap<AActor*, AShipPassangerGhostCore*> ActivePassengers;

	UPROPERTY()
	TSet<TWeakObjectPtr<ACharacter>> RecentlyBoarded;

	UFUNCTION()
	void OnPassengerDied(AActor* DeadActor);
};