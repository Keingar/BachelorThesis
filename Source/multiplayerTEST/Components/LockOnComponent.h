// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "multiplayerTEST/CustomStructs/SocketInfo.h"
#include "multiplayerTEST/CustomStructs/LockOnCameraParams.h"
#include "LockOnComponent.generated.h"

class AmultiplayerTESTCharacter;
class UMySpringArmComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTEST_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULockOnComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool isLockedOn;
	bool canChangeTargetAgain;

	static constexpr float LookupTable[3][3] =
	{
		{   0.f,    0.f,     180.f },   
		{  90.f,   45.f,     135.f },   
		{ -90.f,  -45.f,    -135.f }  
	};
	static constexpr float DefaultArmLength = 400.f;
	static constexpr float DefaultSocketZ = 0.f;
	static constexpr float ZoomInterpSpeed = 4.0f;

	FTimerHandle StopLockOnByWalls;
	FTimerHandle StopLockOnByDistance;
	FTimerHandle CheckIfLockOnIsValid;
	FTimerHandle LockOnSwitchTargetTimer;
	FTimerHandle TimerToChangeTargetAgain;

	// current target info
	AActor* LockOnTarget;
	USkeletalMeshComponent* LockOnTargetMesh;
	FSocketInfo LockOnSocket;
	FLockOnCameraParams CurrentLockOnCameraParams;
	bool bUsingCustomCameraParams;

	AmultiplayerTESTCharacter* OwnerRef;
	UMySpringArmComponent* OwnerCamera;
	AController* OwnerController;
public:
	void StopLockOn();

	void ClearPreviousLockOn();

	UFUNCTION()
	void StopLockOnAfterTargetDeath(AActor* DeadTarget);

	void TryLockOn(int XDirection, int YDirection);

	void SetOwnerController(AController* NewController) { OwnerController = NewController; }
	bool GetCanChangeTargetAgain() const { return canChangeTargetAgain; }

	UFUNCTION(BlueprintCallable)
	bool GetIsLockedOn() const { return isLockedOn; }
	UFUNCTION(BlueprintCallable)
	FVector GetLockOnSocketTargetLocation();

protected:
	float GetZValue(int XDirection, int YDirection) const;

	FRotator GetSphereRotator(float SphereAngle, int YDirection) const;

	FVector GetSpherePosition(int YDirection) const;

	void GetLockOnInfo(const TArray<FHitResult>& InResults, FVector InSphereCenter, int InYDirection, FSocketInfo& OutName, AActor*& OutActor) const;

	FVector GetLockOnLocationVectorCheck() const;

	void CheckDistanceAndWalls();

	void CalculationOfDirection();

	void CanChangeTargetAgain();
protected:
	UFUNCTION(Server, Reliable)
	void SetIsLockedOnServer(bool NewIsLockedOn);
	void SetIsLockedOn(bool NewIsLockedOn);

	UFUNCTION(Server, Reliable)
	void SetLockOnTargetInfoServer(AActor* NewLockOnTarget, FSocketInfo newLockOnSocket);
	void SetLockOnTargetInfo(AActor* NewLockOnTarget, FSocketInfo newLockOnSocket);

protected:
	// list of setting for component
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.1", ClampMax = "5"))
	float XAxisSwitchSensivity;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.1", ClampMax = "5"))
	float YAxisSwitchSensivity;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin ="0", ClampMax= "1"))
	float DelayForSwitchAfterSwitch;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "500", ClampMax = "5000"))
	float MaxLockOnDistance;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "300", ClampMax = "2000"))
	float LockOnCheckSphereRadius;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100"))
	float MinDistanceToSwitch2D;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "-60", ClampMax = "-25"))
	float MaxDownPitch;
};
