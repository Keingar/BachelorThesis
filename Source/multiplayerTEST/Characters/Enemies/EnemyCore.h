// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "multiplayerTEST/GameplayInterfaces/CombatInterface.h"
#include "multiplayerTEST/GameplayInterfaces/LockOnInterface.h"
#include "multiplayerTEST/GameplayInterfaces/EnemyControlInterface.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/TagManagerOwner.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/GhostAnimInstanceSyncInterface.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/GetOwningMainMesh.h"
#include "multiplayerTEST/CustomStructs/FormationsAI/FormationDecision.h"
#include "multiplayerTEST/CustomStructs/SocketInfo.h"
#include "EnemyCore.generated.h"

class UHealthComponent;
class ULootManagerComponent;
class UGameplayTagManagerComp;
class UWidgetComponent;
class UImpactComponent;
class UEnemyTargetingComponent;
class UUserWidget;
class AShipEnemyPassangerGhost;
class UGroupCommandComponent;

UCLASS()
class MULTIPLAYERTEST_API AEnemyCore : public ACharacter,
	public ICombatInterface,
	public ILockOnInterface, 
	public ITagManagerOwner,
	public IEnemyControlInterface,
	public IGhostAnimInstanceSyncInterface,
	public IGetOwningMainMesh
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (AllowPrivateAccess = "true"))
	ULootManagerComponent* LootManagerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (AllowPrivateAccess = "true"))
	UGameplayTagManagerComp* TagsManagerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* LockOnPointt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* Healthbar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = "true"))
	UImpactComponent* ImpactComp;

public:
	AEnemyCore();

	UFUNCTION(BlueprintPure, Category = "EnemyAttributes")
	FORCEINLINE FText GetEnemyName() const { return EnemyName; }

	UFUNCTION(BlueprintPure, Category = "EnemyAttributes")
	FORCEINLINE bool GetIsBoss() const { return bisBoss; }

	UFUNCTION(BlueprintPure, Category = "Formation")
	EFormationMemberRole GetPreferredFormationRole() const { return PreferredFormationRole; }

	UFUNCTION(BlueprintPure, Category = "Formation")
	EFormationMemberRole GetCurrentFormationRole() const { return CurrentFormationRole; }

	UFUNCTION(BlueprintCallable, Category = "Formation")
	void SetPreferredFormationRole(EFormationMemberRole NewRole)
	{
		PreferredFormationRole = NewRole;
		CurrentFormationRole = NewRole;
	}

	void SetCurrentFormationRole(EFormationMemberRole NewRole) { CurrentFormationRole = NewRole; }

//	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "LockOn")
	bool CanBeLockedOn_Implementation() const override;

	UFUNCTION(BlueprintCallable)
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	//UFUNCTION()
	virtual const TArray<FSocketInfo> GetLockOnSocketNames_Implementation() const override;

	virtual USkeletalMeshComponent* GetLockOnTargetMesh_Implementation() const override;

	virtual void OnLockedOnByActor_Implementation(FName SelectedSocket) override;

	virtual void StopLockOn_Implementation() override;

	UFUNCTION(BlueprintCallable) // to have consistant base location 
	void SetBaseBP(UPrimitiveComponent* NewBaseComp);

	virtual UGameplayTagManagerComp* GetTagManager_Implementation()const override { return TagsManagerComponent ;}

	virtual TArray<FLockOnCameraParams> GetLockOnCameraParams() override { return LockOnCameraParams; }

	virtual void SetRotationSpeed_Implementation(float NewRotationSpeed) override;
	virtual void SpawnDecalEffectForStaggerHitBox_Implementation() override;
	virtual void RemoveDecalEffectForStaggerHitBox_Implementation() override;

	virtual TSubclassOf<UAnimInstance> GetGhostAnimInstanceClass() const override { return GhostAnimInstanceSync; }
	virtual ACharacter* GetPossessedGhostCharacter_Implementation() const override;

	virtual UMeshComponent* GetOwningMainMesh_Implementation() const override { return GetMesh(); }

	virtual bool StartFight_Implementation(AActor* EncounterInitiator) override;
	virtual bool bCanStartFight(AActor* Enemy) const override;

	UPROPERTY(BlueprintReadOnly)
	AShipEnemyPassangerGhost* PossessedGhost; // valid only on ship

	UPROPERTY()
	UEnemyTargetingComponent* OwnerTargetComp;

	// Pointer to group command component if this enemy is part of a group
	UPROPERTY()
	UGroupCommandComponent* OwningGroupCommandComponent;
protected:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime)override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void Destroyed() override;

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	TArray<FString> GetSocketNames() const; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	TArray<FSocketInfo> LockOnSocketsInfo;

	TSubclassOf<UUserWidget> LockOnPointWidgetClass;

	TSubclassOf<UUserWidget> EnemyHealthBarClass;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	TArray<FLockOnCameraParams> LockOnCameraParams;

	// needed to not copy entire anim instance of real character  
	// since anim instance is neeeded only to copy montages root motion
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAnimInstance> GhostAnimInstanceSync;

protected:

	UPROPERTY(EditAnywhere, Category = "Enemy")
	FText EnemyName;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	bool bisBoss;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation")
	EFormationMemberRole PreferredFormationRole = EFormationMemberRole::Defender;

	UPROPERTY(BlueprintReadOnly, Category = "Formation")
	EFormationMemberRole CurrentFormationRole = EFormationMemberRole::Defender;

	float RotationSpeed ; 

	UDecalComponent* StaggerAttackZoneDecalComp; // save pointer to remove after spawning 

public:
	bool bCanStartFight()const;

	bool bNeedReset;
	void OverlapActorsAtSpawn();

protected:

	UFUNCTION()
	void Death(AActor* DiedActor);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDied();

};
