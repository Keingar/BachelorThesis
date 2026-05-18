// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCore.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "multiplayerTEST/Components/EnemyComponents/LootManagerComponent.h"
#include "Components/WidgetComponent.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/Components/ImpactComponent.h"
#include "multiplayerTEST/UI/Health/WorldSpaceHealthBar.h"
#include "Components/CapsuleComponent.h"
#include "multiplayerTEST/Controllers/CoreAIController.h"
#include "multiplayerTEST/Components/EnemyComponents/EnemyTargetingComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "multiplayerTEST/GameplayInterfaces/Interactables/SpawnOverlapHandleInterface.h"
#include "multiplayerTEST/Characters/OceanCharacter/Passengers/ShipEnemyPassangerGhost.h"

AEnemyCore::AEnemyCore()
{
	HealthComponent = CreateDefaultSubobject<UHealthComponent>("HealthComp");

	LootManagerComponent = CreateDefaultSubobject<ULootManagerComponent>("LootManagerComp");

	static ConstructorHelpers::FClassFinder<UUserWidget> isLockOnWidgetClass(TEXT("/Game/UI/LockOnPoint"));

	if (isLockOnWidgetClass.Succeeded())
	{
		LockOnPointWidgetClass = isLockOnWidgetClass.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> isEnemyHealthBarWidgetClass(TEXT("/Game/UI/HealthBars/W_EnemyHealthbar"));

	if (isEnemyHealthBarWidgetClass.Succeeded())
	{
		EnemyHealthBarClass = isEnemyHealthBarWidgetClass.Class;
	}

	bisBoss = false;
	bNeedReset = false;
 
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);

	LockOnPointt = CreateDefaultSubobject<UWidgetComponent>("OmegatestCompnentt");
	LockOnPointt->SetupAttachment(GetMesh());
	LockOnPointt->SetWidgetClass(LockOnPointWidgetClass);
	LockOnPointt->SetDrawSize(FVector2D(40, 40));
	LockOnPointt->SetWidgetSpace(EWidgetSpace::Screen);
	LockOnPointt->SetVisibility(false);

	Healthbar = CreateDefaultSubobject<UWidgetComponent>("Healthbar");
	Healthbar->SetupAttachment(GetMesh());
	Healthbar->SetWidgetClass(EnemyHealthBarClass);
	Healthbar->SetDrawSize(FVector2D(200, 12));
	Healthbar->SetWidgetSpace(EWidgetSpace::Screen);
	Healthbar->SetVisibility(false);

	TagsManagerComponent = CreateDefaultSubobject<UGameplayTagManagerComp>("TagsManagerComp");
	ImpactComp = CreateDefaultSubobject<UImpactComponent>("ImpactComp");

	RotationSpeed = 0;

}

void AEnemyCore::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) {
		HealthComponent->ActorDied.AddDynamic(this, &AEnemyCore::Death);
	}

	UMaterialInterface* Mat = ImpactComp->GetDecalMaterialForStaggerZone();
	if (Mat)
	{
		// Touch material to force shader warm-up
		Mat->CacheShaders();
	}		
}

void AEnemyCore::OverlapActorsAtSpawn() {
	if (!HasAuthority())
		return;

	GetCapsuleComponent()->UpdateOverlaps();

	TArray<AActor*> OverlappingActors;
	GetCapsuleComponent()->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor || Actor == this)
			continue;

		if (Actor->GetClass()->ImplementsInterface(USpawnOverlapHandleInterface::StaticClass()))
		{
			ISpawnOverlapHandleInterface::Execute_BeginOverlapAtSpawn(Actor, this);
		}
	}
}


void AEnemyCore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!OwnerTargetComp || RotationSpeed == 0) return;

	AActor* CurrentTarget = OwnerTargetComp->GetCurrentTarget();

	if (!CurrentTarget) return;

	FRotator DesiredRotation = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D().Rotation();

	SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), DesiredRotation, DeltaTime, RotationSpeed));
}

void AEnemyCore::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AEnemyCore::Destroyed()
{
	if(PossessedGhost) PossessedGhost->Destroy();

	Super::Destroyed();
}

bool AEnemyCore::CanBeLockedOn_Implementation() const
{
	if (!HealthComponent) return false;

	return !HealthComponent->GetIsDead();
}

const TArray<FSocketInfo> AEnemyCore::GetLockOnSocketNames_Implementation() const
{
	return LockOnSocketsInfo;
}

USkeletalMeshComponent* AEnemyCore::GetLockOnTargetMesh_Implementation() const
{
	return GetMesh();
}

TArray<FString> AEnemyCore::GetSocketNames() const
{
	TArray<FString> SocketNames;

	USkeletalMeshComponent* SkeletalMeshComp = GetMesh(); 
	if (SkeletalMeshComp)
	{
		TArray<FName> SocketNamesFName;
		SocketNamesFName = SkeletalMeshComp->GetAllSocketNames();

		for (const FName& SocketName : SocketNamesFName)
		{
			SocketNames.Add(SocketName.ToString());
		}
	}

	return SocketNames;
}

void AEnemyCore::Death(AActor* DiedActor)
{
	if (UAnimInstance* MeshAnimInstance = GetMesh()->GetAnimInstance()) {
		MeshAnimInstance->StopAllMontages(0);
	}

	OnDied();
}

bool AEnemyCore::bCanStartFight() const
{
	if (!HealthComponent) {
		return false;
	}

	return !HealthComponent->GetIsDead();
}

bool AEnemyCore::StartFight_Implementation(AActor* EncounterInitiator)
{
	if (!HasAuthority() || !EncounterInitiator)
	{
		return false;
	}

	ACoreAIController* AIController = Cast<ACoreAIController>(GetController());
	if (!AIController)
	{
		return false;
	}

	return AIController->CheckSensedActor(EncounterInitiator);
}

bool AEnemyCore::bCanStartFight(AActor* Enemy) const
{
	if (!Enemy || Enemy == this)
	{
		return false;
	}

	return bCanStartFight();
}

void AEnemyCore::OnLockedOnByActor_Implementation(FName SelectedSocket)
{
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);

	LockOnPointt->AttachToComponent(GetMesh(), AttachmentRules, SelectedSocket);

	LockOnPointt->SetVisibility(true);

	Cast<UWorldSpaceHealthBar>(Healthbar->GetWidget())->SetLockedOn(true);
}

void AEnemyCore::StopLockOn_Implementation()
{
	LockOnPointt->SetVisibility(false);

	Cast<UWorldSpaceHealthBar>(Healthbar->GetWidget())->SetLockedOn(false);

}
void AEnemyCore::SetBaseBP(UPrimitiveComponent* NewBaseComp)
{
	SetBase(NewBaseComp);
}


void AEnemyCore::SetRotationSpeed_Implementation(float NewRotationSpeed)
{
	RotationSpeed = NewRotationSpeed;
	if(PossessedGhost) PossessedGhost->RotationSpeed = NewRotationSpeed;
}

void AEnemyCore::SpawnDecalEffectForStaggerHitBox_Implementation()
{
	if (StaggerAttackZoneDecalComp) {
		UE_LOG(LogTemp, Error, TEXT("Tried to spawn already spawned decal for stagger hit zone"));
	}

	if (!ImpactComp || !ImpactComp->GetDecalMaterialForStaggerZone() || !GetCapsuleComponent())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	FVector DecalWorldLocation =
		GetActorTransform().TransformPosition(ImpactComp->GetStaggerCritHitBoxCenter());

	// Correct decal projection rotation
	FRotator DecalRotation = FRotator(-90.f, 0.f, 0.f);

	FVector DecalSize(
		50.f,
		ImpactComp->GetStaggerCritHitBoxRadius(),
		ImpactComp->GetStaggerCritHitBoxRadius()
	);

	StaggerAttackZoneDecalComp = UGameplayStatics::SpawnDecalAttached(
		ImpactComp->GetDecalMaterialForStaggerZone(),
		DecalSize,
		GetCapsuleComponent(),
		NAME_None,
		DecalWorldLocation,
		DecalRotation,
		EAttachLocation::KeepWorldPosition,
		0.f
	);

	if (!StaggerAttackZoneDecalComp)
		return;

	StaggerAttackZoneDecalComp->SetVisibility(false, true);

	StaggerAttackZoneDecalComp->SetFadeIn(
		0,    // no delay
		1    // fade duration
	);

	StaggerAttackZoneDecalComp->SetVisibility(true, true);


}

void AEnemyCore::RemoveDecalEffectForStaggerHitBox_Implementation()
{
	if (!StaggerAttackZoneDecalComp ) {return;}

	StaggerAttackZoneDecalComp->SetFadeOut(0.f, 0.5f, false);

	// Destroy the component after fade duration
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			if (StaggerAttackZoneDecalComp)
			{
				StaggerAttackZoneDecalComp->DestroyComponent();
				StaggerAttackZoneDecalComp = nullptr;
			}
		}, 0.5f, false);
	
}

ACharacter* AEnemyCore::GetPossessedGhostCharacter_Implementation() const
{
	return PossessedGhost;
}
