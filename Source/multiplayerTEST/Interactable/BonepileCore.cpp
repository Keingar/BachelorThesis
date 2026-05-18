// Fill out your copyright notice in the Description page of Project Settings.

#include "BonepileCore.h"
#include "multiplayerTEST/AnimInstances/PlayerAnimInstance.h"
#include "multiplayerTEST/Characters/multiplayerTESTCharacter.h"
#include "Components/CapsuleComponent.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/SaveGameSubsystem.h"
#include "multiplayerTEST/Components/OtherComponents/ObjectRegistryComponent.h"
#include "Engine/GameInstance.h"

ABonepileCore::ABonepileCore()
{
	needReset = false;
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	MeshRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MeshRoot"));

	MeshRoot->SetupAttachment(RootComponent);

	// Create mesh
	BonfireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BonfireMesh"));
	BonfireMesh->SetupAttachment(MeshRoot);

	DestinationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMesh"));
	DestinationMesh->SetupAttachment(MeshRoot);
	// Create sphere
	InteractionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InteractionRadiusCapsule"));
	InteractionCapsule->SetupAttachment(RootComponent);
	InteractionCapsule->SetUsingAbsoluteRotation(true);

	// Setup sphere size
	InteractionCapsule->InitCapsuleSize(50, 100);

	// Collision settings
	InteractionCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCapsule->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionCapsule->SetGenerateOverlapEvents(true);

	// Bind overlap events
	InteractionCapsule->OnComponentBeginOverlap.AddDynamic(this, &ABonepileCore::OnCapsuleBeginOverlap);
	InteractionCapsule->OnComponentEndOverlap.AddDynamic(this, &ABonepileCore::OnCapsuleEndOverlap);
}


void ABonepileCore::Interact_Implementation(AActor* UserActor)
{
	AmultiplayerTESTCharacter* Character = Cast<AmultiplayerTESTCharacter>(UserActor);
	if (!Character) return;

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh) return;

	UPlayerAnimInstance* AnimInstance = Cast<UPlayerAnimInstance>(Mesh->GetAnimInstance());
	if (!AnimInstance) return;

	if (AnimInstance->bIsChangingRestState) return;

	if (Character->GetTagManager_Implementation() && Character->GetTagManager_Implementation()->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.RestrictiveEffects.Falling")))) return;

	if (bInteractingWithBonfire) return;
	bInteractingWithBonfire = true;

	if (!AnimInstance->bWantToRestAtBonfire) {
		TryToSaveBonpile();
		Character->OnStartBonfireRest();
	}
	else {
		Character->OnStopBonfireRestStarted();
	}

	AnimInstance->bWantToRestAtBonfire = !AnimInstance->bWantToRestAtBonfire;
	bInteractingWithBonfire = false;
}

FName ABonepileCore::GetBonepileID() const
{
	return RegistryComponent ? RegistryComponent->ObjectID : NAME_None;
}


void ABonepileCore::ResetObject_Implementation()
{
	// no need for reset for now
}

void ABonepileCore::OnCapsuleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	AmultiplayerTESTCharacter* Character = Cast<AmultiplayerTESTCharacter>(OtherActor);
	if (!Character) return;

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh) return;

	UPlayerAnimInstance* AnimInstance = Cast<UPlayerAnimInstance>(Mesh->GetAnimInstance());
	if (!AnimInstance) return;

	Character->LastOverlappedBonfireID = GetBonepileID();

	Character->CurrentlyOverlappedBonfire = this;

	AnimInstance->bInBonfireInteractionRadius = true;

	Character->CallDrawCanRestAtBonfireWidget();
}

void ABonepileCore::OnCapsuleEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	AmultiplayerTESTCharacter* Character = Cast<AmultiplayerTESTCharacter>(OtherActor);
	if (!Character) return;

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh) return;

	UPlayerAnimInstance* AnimInstance = Cast<UPlayerAnimInstance>(Mesh->GetAnimInstance());
	if (!AnimInstance) return;
	
	Character->CurrentlyOverlappedBonfire = nullptr;

	AnimInstance->bInBonfireInteractionRadius = false;

	Character->CallCloseCanRestAtBonfireWidget();
}

bool ABonepileCore::TryToSaveBonpile()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return false;
	}

	USaveGameSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USaveGameSubsystem>();
	if (!SaveSubsystem)
	{
		return false;
	}

	const FName BonepileID = GetBonepileID();
	if (BonepileID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("BonepileCore %s has no valid RegistryComponent ObjectID; skipping save entry"), *GetName());
		return false;
	}

	// Record bonpile without immediate save; then force-save once.
	SaveSubsystem->SetLastRestedBonpile(BonepileID);
	SaveSubsystem->AddOpenedBonpile(BonepileID);
	SaveSubsystem->ForceSave();

	return true;
}

FVector ABonepileCore::GetTeleportLocation() const
{
	return DestinationMesh->GetComponentLocation();
}

FRotator ABonepileCore::GetTeleportRotation() const
{
	return DestinationMesh->GetComponentRotation();
}