#include "ShipPlayerPassengerGhost.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "multiplayerTEST/Components/PlayerComponents/MultiCharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "multiplayerTEST/Characters/multiplayerTESTCharacter.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/Components/PlayerComponents/MySpringArmComponent.h"
#include "multiplayerTEST/Characters/OceanCharacter/Ships/Ship_Ghost.h"
#include "multiplayerTEST/Characters/OceanCharacter/Ships/Ship_Visual.h"

AShipPlayerPassengerGhost::AShipPlayerPassengerGhost(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMultiCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	CameraBoom = CreateDefaultSubobject<UMySpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, UMySpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

}

void AShipPlayerPassengerGhost::AttachCameraToActor(AActor* TargetActor)
{
	if (!CameraBoom || !TargetActor || !TargetActor->GetRootComponent())
	{
		return;
	}

	CameraBoom->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	CameraBoom->AttachToComponent(
		TargetActor->GetRootComponent(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);
}

void AShipPlayerPassengerGhost::CopySettingsFromActor(ACharacter* RealCharacter)
{
	Super::CopySettingsFromActor(RealCharacter);

	// here only camera copy for player
	if (!RealCharacter || !CameraBoom || !FollowCamera)
	{
		return;
	}

	AmultiplayerTESTCharacter* RealPlayer = Cast<AmultiplayerTESTCharacter>(RealCharacter);
	if (!RealPlayer)
	{
		return;
	}

	UMySpringArmComponent* SourceBoom = RealPlayer->GetCameraBoom();
	UCameraComponent* SourceCamera = RealPlayer->GetFollowCamera();

	if (!SourceBoom || !SourceCamera)
	{
		return;
	}

	// Spring arm settings
	CameraBoom->TargetArmLength = SourceBoom->TargetArmLength;
	CameraBoom->SocketOffset = SourceBoom->SocketOffset;
	CameraBoom->TargetOffset = SourceBoom->TargetOffset;
	CameraBoom->ProbeSize = SourceBoom->ProbeSize;
	CameraBoom->ProbeChannel = SourceBoom->ProbeChannel;
	CameraBoom->bDoCollisionTest = SourceBoom->bDoCollisionTest;
	CameraBoom->bUsePawnControlRotation = SourceBoom->bUsePawnControlRotation;
	CameraBoom->bEnableCameraLag = SourceBoom->bEnableCameraLag;
	CameraBoom->CameraLagSpeed = SourceBoom->CameraLagSpeed;
	CameraBoom->CameraLagMaxDistance = SourceBoom->CameraLagMaxDistance;
	CameraBoom->bEnableCameraRotationLag = SourceBoom->bEnableCameraRotationLag;
	CameraBoom->CameraRotationLagSpeed = SourceBoom->CameraRotationLagSpeed;
	CameraBoom->CameraLagMaxTimeStep = SourceBoom->CameraLagMaxTimeStep;
	CameraBoom->bInheritPitch = SourceBoom->bInheritPitch;
	CameraBoom->bInheritYaw = SourceBoom->bInheritYaw;
	CameraBoom->bInheritRoll = SourceBoom->bInheritRoll;
	CameraBoom->MaxDownDistance = SourceBoom->MaxDownDistance;

	// Camera settings
	FollowCamera->FieldOfView = SourceCamera->FieldOfView;
	FollowCamera->PostProcessBlendWeight = SourceCamera->PostProcessBlendWeight;
	FollowCamera->SetRelativeLocation(SourceCamera->GetRelativeLocation());
	FollowCamera->SetRelativeRotation(SourceCamera->GetRelativeRotation());
}

void AShipPlayerPassengerGhost::BeginPlay()
{
	Super::BeginPlay();

	TryAddMappingContext();
}

void AShipPlayerPassengerGhost::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	TryAddMappingContext();
}

void AShipPlayerPassengerGhost::UnPossessed()
{
	Super::UnPossessed();

	Destroy();
}

void AShipPlayerPassengerGhost::OnRep_Controller()
{
	Super::OnRep_Controller();
	TryAddMappingContext();
}

void AShipPlayerPassengerGhost::TryAddMappingContext()
{
	APlayerController* PC = Cast<APlayerController>(Controller);

	if (!PC || !PC->IsLocalController() || !DefaultMappingContext)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if (!Subsystem->HasMappingContext(DefaultMappingContext))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AShipPlayerPassengerGhost::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShipPlayerPassengerGhost::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &AShipPlayerPassengerGhost::StartMove);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AShipPlayerPassengerGhost::StopMove);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShipPlayerPassengerGhost::Look);


		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AShipPlayerPassengerGhost::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AShipPlayerPassengerGhost::StopSprint);

		EnhancedInputComponent->BindAction(OpenEscMenuAction, ETriggerEvent::Started, this, &AShipPlayerPassengerGhost::OpenEscMenu);
		EnhancedInputComponent->BindAction(TwoHandAction, ETriggerEvent::Started, this, &AShipPlayerPassengerGhost::TryTwoHand);

		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AShipPlayerPassengerGhost::LightAttack);
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AShipPlayerPassengerGhost::HeavyAttack);

		EnhancedInputComponent->BindAction(DoubleLightAttackAction, ETriggerEvent::Started, this, &AShipPlayerPassengerGhost::DoubleLightAttackOrBlock);
		EnhancedInputComponent->BindAction(DoubleLightAttackAction, ETriggerEvent::Completed, this, &AShipPlayerPassengerGhost::TryEndBlock);

		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &AShipPlayerPassengerGhost::Roll);

		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &AShipPlayerPassengerGhost::LockOn);

	}
}

void AShipPlayerPassengerGhost::Move(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();

	if (VisualShip && GhostShip)
	{
		const float DeltaYaw =
			(GhostShip->GetActorRotation().Yaw - VisualShip->GetActorRotation().Yaw);

		FVector WorldInput(Input.Y, Input.X, 0.f);

		const FVector Adjusted =
			WorldInput.RotateAngleAxis(DeltaYaw, FVector::UpVector);

		Input = FVector2D(Adjusted.Y, Adjusted.X);
	}

	RealActorPlayer->MoveCharacter(Input, this);
}

void AShipPlayerPassengerGhost::StartMove(const FInputActionValue& Value)
{
	RealActorPlayer->StartMoveFunc();
}

void AShipPlayerPassengerGhost::StopMove(const FInputActionValue& Value)
{
	RealActorPlayer->StopMoveFunc();
}

void AShipPlayerPassengerGhost::Look(const FInputActionValue& Value)
{
	RealActorPlayer->LookCharacter(Value, this);
}

void AShipPlayerPassengerGhost::StartSprint(const FInputActionValue& Value)
{
	RealActorPlayer->StartSprintFunc();
}

void AShipPlayerPassengerGhost::StopSprint(const FInputActionValue& Value)
{
	RealActorPlayer->StopSprintFunc();
}

void AShipPlayerPassengerGhost::OpenEscMenu(const FInputActionValue& Value)
{
	RealActorPlayer->OpenEscMenuFunc();
}

void AShipPlayerPassengerGhost::TryTwoHand(const FInputActionValue& Value)
{
	RealActorPlayer->TryTwoHandFunc();
}

void AShipPlayerPassengerGhost::LightAttack(const FInputActionValue& Value)
{
	RealActorPlayer->TryLightAttackFunc();
}

void AShipPlayerPassengerGhost::HeavyAttack(const FInputActionValue& Value)
{
	RealActorPlayer->TryHeavyAttackFunc();
}

void AShipPlayerPassengerGhost::DoubleLightAttackOrBlock(const FInputActionValue& Value)
{
	RealActorPlayer->TryDoubleLightAttackOrBlockFunc();
}

void AShipPlayerPassengerGhost::TryEndBlock(const FInputActionValue& Value)
{
	RealActorPlayer->OnEndBlock(Value);
}

void AShipPlayerPassengerGhost::Roll(const FInputActionValue& Value)
{
	RealActorPlayer->TryRollFunc();
}

void AShipPlayerPassengerGhost::LockOn(const FInputActionValue& Value)
{
	RealActorPlayer->LockOn(Value);
}