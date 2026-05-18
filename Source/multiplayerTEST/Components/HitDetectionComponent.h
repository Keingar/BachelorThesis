// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "multiplayerTEST/CustomStructs/HitboxData.h"
#include "multiplayerTEST/CustomStructs/DamageInfo.h"
#include "GameplayTagContainer.h"
#include "HitDetectionComponent.generated.h"

class UWeapon;
class UGameplayTagManagerComp;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHitDetected, const FHitResult&, HitActor,const FDamageInfo&, HitDamageInfo);

USTRUCT(BlueprintType)
struct FHitBoxGroup
{
	GENERATED_BODY()

public:

	// A short description for this set of hitboxes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox")
	FString Description;

	// The actual hitbox data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox")
	TArray<FHitBoxData> HitBoxes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox", meta = (GetOptions = "GetSocketNames"))
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox")
	FDamageInfo currentHitBoxDamageInfo;

	FCollisionQueryParams QueryParams;
	FTimerHandle HitBoxCollisionTimer;

	FVector PreviousSocketLocation; // used for sweep trace functions to not have gaps 
	bool bUsePreviousSocketDetection; // used to make it ignore previous location in the very first detection call
};

struct FFullWeaponHitBox
{
	FHitBoxGroup HitBoxesGroup;

	USceneComponent* WeaponComponent;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTEST_API UHitDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHitDetectionComponent();
	
	void GetNewWeaponData(UWeapon* NewWeapon, USceneComponent* SelectedWeapon);

	UFUNCTION(BlueprintCallable)
	void StartNewSocketCollision(int HitBoxChoice);
	UFUNCTION(BlueprintCallable)
	void StopSocketCollision(int HitBoxChoice);

	UFUNCTION(BlueprintCallable)
	void DoSocketCollisionOnce(int HitBoxChoice);

	UFUNCTION(BlueprintCallable)
	void StartWeaponCollision(USceneComponent* SelectedWeapon, FDamageInfo AttackDamageInfo);

	UFUNCTION(BlueprintCallable)
	void StopWeaponCollision(USceneComponent* SelectedWeapon);

	// for now used only for stagger hits
	TArray<AActor*> GetActorsInFrontOfOwner();
protected:
	virtual void BeginPlay() override;

	UMeshComponent* OwnerMesh;
	UGameplayTagManagerComp* OwnerTagComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox")
	TArray<FHitBoxGroup> SocketHitBoxData;

	TArray<FFullWeaponHitBox>  AllWeaponsFullHitBoxes;

	void SetUpOwnerRef();
	void SetUpOwnerTagManagerRef();

	UPROPERTY(EditDefaultsOnly)
	bool DebugCollision = 0;
public:	
	UPROPERTY(BlueprintAssignable, Category = "Hit Detection")
	FHitDetected OnHitDetected;
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	TArray<FString> GetSocketNames() const;

protected:

	void PerformCollisionDetection(int HitBoxChoice);

	void PerformCollisionDetectionForWeapon(int ChosenWeapon);

	void OnNewHit(const FHitResult& DetectedHit, FDamageInfo HitDamage);

	void DrawDebugCollision(FVector DebugHitBoxStartLocation, FVector DebugHitBoxEndLocation, FCollisionShape DebugHitBoxShape, FHitBoxData DebugHitBoxData, FQuat DebugHitBoxRotation);

	FGameplayTagContainer TagsThatBlockHit;
};
