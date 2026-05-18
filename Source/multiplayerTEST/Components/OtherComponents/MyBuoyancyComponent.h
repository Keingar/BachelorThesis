// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "multiplayerTEST/OceanWater/OceanWaterZone.h"
#include "MyBuoyancyComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIPLAYERTEST_API UMyBuoyancyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMyBuoyancyComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy Data")
	float PitchStrength = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy Data")
	float RollStrength = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy Data")
	TArray<FVector> BouyancyPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy Data")
	bool DebugPoints = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy Data")
	UStaticMeshComponent* MyStaticMeshComponent;

	UFUNCTION(BlueprintCallable, Category = "Buoyancy")
	FVector GetBuoyancyLocation(FVector RelativeLocation);
	UFUNCTION(BlueprintCallable, Category = "Buoyancy")
	FVector GetMultiBuoyancyLocation(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "Buoyancy")
	TArray<FVector> GetBuoyancyArray(TArray<FVector> Points);


private:
	AActor* ParentActor = nullptr;
	AOceanWaterZone* OceanWaterZone;
	FOceanFFTCalculator* FFTCalculator;
	bool bWaterZoneValid = true;
	FTransform ActorNotMovingTranform; // when actor is not moving its not changing (for displacement accuracy)
	

	FVector SmoothedTorque = FVector::ZeroVector;
	FRotator SmoothedTargetRotation;
	FVector PreviousTorque = FVector::ZeroVector;
	float SmoothedResponseSpeed;
	FRotator PreviousRotation;

	FOceanFFTCalculator* InitializeWaterZoneReference();
	UFUNCTION(BlueprintCallable)
	void FindAverageLocationofPontoons();
	void RotateOwner(float DeltaTime);
	void DrawBuoyancyArrayDebugPoints(const TArray<FVector>& BuoyancyArray);
	
	FVector AverageLocationofPontoons = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly)
	float artificalAdditionalHeight;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	FVector GetNewMovingPosition(float DeltaTime);

	UPROPERTY(BlueprintReadWrite, Category = "Buoyancy Data")
	float SpeedPerSecond;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
