#include "Ship_Visual.h"
#include "Ship_Ghost.h"
#include "multiplayerTEST/Characters/OceanCharacter/Passengers/ShipPlayerPassengerGhost.h"
#include "multiplayerTEST/Characters/OceanCharacter/Passengers/ShipEnemyPassangerGhost.h"
#include "multiplayerTEST/Components/OtherComponents/MyBuoyancyComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "multiplayerTEST/Characters/multiplayerTESTCharacter.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/GhostAnimSyncInterface.h"
#include "multiplayerTEST/Components/PlayerComponents/SprintComponent.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "multiplayerTEST/Components/ImpactComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "multiplayerTEST/Characters/Enemies/EnemyCore.h"

AShip_Visual::AShip_Visual()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	ShipMesh->SetupAttachment(Root);
	ShipMesh->SetMobility(EComponentMobility::Movable);
	ShipMesh->SetCollisionProfileName(TEXT("BlockAll"));

	BoardingVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BoardingVolume"));
	BoardingVolume->SetupAttachment(ShipMesh);
	BoardingVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoardingVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoardingVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	BuoyancyComponent = CreateDefaultSubobject<UMyBuoyancyComponent>(TEXT("BuoyancyComponent"));
}

void AShip_Visual::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SyncGhostShipTransform();
}

void AShip_Visual::BeginPlay()
{
	Super::BeginPlay();

	check(GhostShip);
	check(PlayerPassengerGhostClass);

	SyncGhostShipTransform();

	BoardingVolume->OnComponentBeginOverlap.AddDynamic(
		this,
		&AShip_Visual::OnBoardingVolumeBeginOverlap
	);

	BoardingVolume->OnComponentEndOverlap.AddDynamic(
		this,
		&AShip_Visual::OnBoardingVolumeEndOverlap
	);
}

void AShip_Visual::SyncGhostShipTransform()
{
	if (!GhostShip)
		return;

	//GhostShip->SetActorLocationAndRotation(
		//GetActorLocation(),
		//GetActorRotation(),
	//	false,
	//	nullptr,
	//	ETeleportType::TeleportPhysics
//	);

	GhostShip->SetActorScale3D(GetActorScale3D());
}

void AShip_Visual::BoardPassenger(ACharacter* RealCharacter)
{
	if (!HasAuthority() || !RealCharacter || !GhostShip || !RealCharacter->GetController())
		return;

	if (ActivePassengers.Contains(RealCharacter))
		return;

	AmultiplayerTESTCharacter* PlayerRealChar = Cast<AmultiplayerTESTCharacter>(RealCharacter);
	AEnemyCore* EnemyRealChar = Cast<AEnemyCore>(RealCharacter);

	if (PlayerRealChar && !PlayerRealChar->bSetUpFinished) return;

	const FTransform Local =
		RealCharacter->GetActorTransform().GetRelativeTransform(GetActorTransform());

	const FTransform GhostWorld =
		Local * GhostShip->GetActorTransform();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AShipPassangerGhostCore* Ghost = nullptr;
	AShipPlayerPassengerGhost* PlayerGhost = nullptr; // just to not cast later
	AShipEnemyPassangerGhost* EnemyGhost = nullptr; //same reason
	// player
	if(PlayerRealChar){
		PlayerGhost = GetWorld()->SpawnActor<AShipPlayerPassengerGhost>(
				PlayerPassengerGhostClass,
				GhostWorld.GetLocation(),
				GhostWorld.GetRotation().Rotator(),
				Params
			);

		Ghost = PlayerGhost;
	} //enemy
	else {
		EnemyGhost = GetWorld()->SpawnActor<AShipEnemyPassangerGhost>(
			EnemyPassengerGhostClass,
			GhostWorld.GetLocation(),
			GhostWorld.GetRotation().Rotator(),
			Params
		);

		Ghost = EnemyGhost;

	}

	if (!Ghost)
		return;

	Ghost->VisualShip = this;
	Ghost->GhostShip = GhostShip;
	Ghost->RealActor = RealCharacter;

	SetUpGhostMeshAndAnimInstance(Ghost, RealCharacter);

	// player ref set up
	if (PlayerRealChar && PlayerGhost) {
		PlayerGhost->RealActorPlayer = PlayerRealChar;
		PlayerRealChar->PossessedGhost = PlayerGhost;
		PlayerRealChar->GetSprintComponent()->GhostMovementComp = PlayerGhost->GetCharacterMovement();
		PlayerRealChar->GetImpactComponent()->SetOwnerGhostAnimInstance(PlayerGhost->GetMesh()->GetAnimInstance());
	}

	// enemy ref set up
	if (EnemyRealChar) {
		EnemyRealChar->PossessedGhost = EnemyGhost;
		EnemyGhost->OwnerTargetComp = EnemyRealChar->OwnerTargetComp;
	}

	Ghost->CopySettingsFromActor(RealCharacter);

	SetUpGhostAnimInstanceSync(Ghost, RealCharacter);


	AController* Controller = RealCharacter->GetController();
	if (Controller)
	{
		FRotator SavedRotation = RealCharacter->GetControlRotation();

		Controller->UnPossess();
		Controller->Possess(Ghost);
		Controller->SetControlRotation(SavedRotation);
	}

	RealCharacter->GetCharacterMovement()->DisableMovement();

	if (PlayerGhost) {
		PlayerGhost->TryAddMappingContext();
		PlayerGhost->AttachCameraToActor(RealCharacter);
	}

	ActivePassengers.Add(RealCharacter, Ghost);

	if (UHealthComponent* HealthComp = RealCharacter->FindComponentByClass<UHealthComponent>())
	{
		HealthComp->ActorDied.AddDynamic(this, &AShip_Visual::OnPassengerDied);
	}
	RealCharacter->OnDestroyed.AddDynamic(this, &AShip_Visual::OnPassengerDied);

	RecentlyBoarded.Add(RealCharacter);

	FTimerHandle Timer;
	GetWorldTimerManager().SetTimer(
		Timer,
		[this, RealCharacter]()
		{
			RecentlyBoarded.Remove(RealCharacter);
		},
		0.3f,
		false
	);
}

void AShip_Visual::SetUpGhostMeshAndAnimInstance(AShipPassangerGhostCore* Ghost, ACharacter* RealChar)
{
	if (!Ghost || !RealChar) return;

	UCapsuleComponent* RealCapsule = RealChar->GetCapsuleComponent();
	UCapsuleComponent* GhostCapsule = Ghost->GetCapsuleComponent();

	if (RealCapsule && GhostCapsule)
	{
		GhostCapsule->SetCapsuleSize(
			RealCapsule->GetUnscaledCapsuleRadius(),
			RealCapsule->GetUnscaledCapsuleHalfHeight(),
			true
		);
	}

	USkeletalMeshComponent* RealMesh = RealChar->GetMesh();
	USkeletalMeshComponent* GhostMesh = Ghost->GetMesh();

	if (!RealMesh || !GhostMesh) return;

	IGhostAnimInstanceSyncInterface* AnimSyncInfo = Cast<IGhostAnimInstanceSyncInterface>(RealChar);

	if (!AnimSyncInfo) return;

	if (UAnimInstance* RealAnim = RealMesh->GetAnimInstance())
	{
		RealAnim->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
	}

	GhostMesh->SetSkeletalMesh(RealMesh->GetSkeletalMeshAsset());

	for (int32 i = 0; i < GhostMesh->GetNumMaterials(); ++i)
	{
		GhostMesh->SetMaterial(i, nullptr);
	}

	GhostMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GhostMesh->SetAnimInstanceClass(AnimSyncInfo->GetGhostAnimInstanceClass());

	GhostMesh->SetHiddenInGame(true);
	GhostMesh->SetVisibility(false);
	GhostMesh->SetCastShadow(false);
	GhostMesh->bCastHiddenShadow = false;
	GhostMesh->bReceivesDecals = false;
	GhostMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GhostMesh->bDisableClothSimulation = true;

	GhostMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	GhostMesh->SetRelativeLocation(RealMesh->GetRelativeLocation());
	GhostMesh->SetRelativeRotation(RealMesh->GetRelativeRotation());
	GhostMesh->SetRelativeScale3D(RealMesh->GetRelativeScale3D());
}

void AShip_Visual::UnboardPassenger(ACharacter* RealCharacter)
{
	if (!HasAuthority() || !RealCharacter || !GhostShip)
		return;

	AShipPassangerGhostCore** GhostPtr = ActivePassengers.Find(RealCharacter);
	if (!GhostPtr || !(*GhostPtr))
		return;

	AShipPassangerGhostCore* Ghost = *GhostPtr;

	AController* Controller = Ghost->GetController();
	if (Controller)
	{
		Controller->UnPossess();
		Controller->Possess(RealCharacter);
	}

	RealCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	const FTransform GhostLocal =
		Ghost->GetActorTransform().GetRelativeTransform(GhostShip->GetActorTransform());

	const FTransform FinalWorld =
		GhostLocal * GetActorTransform();

	RealCharacter->SetActorTransform(FinalWorld, false, nullptr, ETeleportType::TeleportPhysics);

	if (UHealthComponent* HealthComp = RealCharacter->FindComponentByClass<UHealthComponent>())
	{
		HealthComp->ActorDied.RemoveDynamic(this, &AShip_Visual::OnPassengerDied);
	}
	RealCharacter->OnDestroyed.RemoveDynamic(this, &AShip_Visual::OnPassengerDied);

	ActivePassengers.Remove(RealCharacter);
}

void AShip_Visual::BeginOverlapAtSpawn_Implementation(AActor* OverlappedActor)
{
	if (!HasAuthority())
		return;

	ACharacter* Character = Cast<ACharacter>(OverlappedActor);
	if (!Character)
		return;
	BoardPassenger(Character);
}

void AShip_Visual::OnBoardingVolumeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority())
		return;

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character)
		return;

	// end overlap is triggered when ragdoll start which happens on death
	UHealthComponent* HealthComp = Character->FindComponentByClass<UHealthComponent>();

	if(!HealthComp || HealthComp->GetIsDead()) 
		return;

	BoardPassenger(Character);
}

void AShip_Visual::OnBoardingVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
		return;

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character)
		return;

	if (RecentlyBoarded.Contains(Character))
		return;

	UHealthComponent* HealthComp = Character->FindComponentByClass<UHealthComponent>();

	if (!HealthComp || HealthComp->GetIsDead())
		return;

	UnboardPassenger(Character);
}

void AShip_Visual::SetUpGhostAnimInstanceSync(AShipPassangerGhostCore* Ghost, ACharacter* RealCharacter)
{
	if (!Ghost || !RealCharacter) return;

	USkeletalMeshComponent* RealCharMesh = RealCharacter->GetMesh();

	if (!RealCharMesh) return;

	IGhostAnimSyncInterface* AnimSync = Cast<IGhostAnimSyncInterface>(RealCharMesh->GetAnimInstance());

	if (!AnimSync) return;
	
	AnimSync->SetGhostMovementComp(Ghost->GetCharacterMovement());
}

void AShip_Visual::OnPassengerDied(AActor* DeadActor)
{
	ACharacter* DeadCharacter = Cast<ACharacter>(DeadActor);
	if (!DeadCharacter)
		return;

	if (UHealthComponent* HealthComp = DeadCharacter->FindComponentByClass<UHealthComponent>())
	{
		HealthComp->ActorDied.RemoveDynamic(this, &AShip_Visual::OnPassengerDied);
	}

	DeadCharacter->OnDestroyed.RemoveDynamic(this, &AShip_Visual::OnPassengerDied);

	ActivePassengers.Remove(DeadCharacter);
	RecentlyBoarded.Remove(DeadCharacter);
}