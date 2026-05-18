// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActorCore.h"
#include "BonepileCore.generated.h"

class UCapsuleComponent;

UCLASS()
class MULTIPLAYERTEST_API ABonepileCore : public AInteractableActorCore
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BonfireMesh;

	// to adjust teleport location a bit for each
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* DestinationMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* MeshRoot;
public:
	ABonepileCore();

	void Interact_Implementation(AActor* UserActor)override;
	void ResetObject_Implementation() override;

	UFUNCTION(BlueprintPure, Category = "Bonfire")
	FName GetBonepileID() const;
	
	// public to call for jump opening
	bool TryToSaveBonpile();

	FVector GetTeleportLocation() const;
	FRotator GetTeleportRotation() const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Bonfire")
	UCapsuleComponent* InteractionCapsule;

	UFUNCTION()
	void OnCapsuleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnCapsuleEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	bool bInteractingWithBonfire; // to solve bugs with double interaction
};
