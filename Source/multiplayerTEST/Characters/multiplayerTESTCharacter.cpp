#include "multiplayerTESTCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTestPlayerState.h"
#include "multiplayerTEST/Components/PlayerComponents/SprintComponent.h"
#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "EnhancedInputSubsystems.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "multiplayerTEST/Components/StaminaComponent.h"
#include "multiplayerTEST/Components/PlayerComponents/RollComponent.h"
#include "multiplayerTEST/Components/PlayerComponents/MySpringArmComponent.h"
#include "multiplayerTEST/Components/HitDetectionComponent.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/Components/InventoryComponent.h"
#include "multiplayerTEST/Components/LockOnComponent.h"
#include "multiplayerTEST/Components/AbilitySystemComponent.h"
#include "multiplayerTEST/Components/PlayerComponents/MultiCharacterMovementComponent.h"
#include "multiplayerTEST/Components/ImpactComponent.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTeestHUD.h"
#include "multiplayerTEST/Items/Weapon.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTestGameStateBase.h"
#include "TimerManager.h"
#include "multiplayerTEST/Controllers/MyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "multiplayerTEST/GameModeRelated/multiplayerTESTGameMode.h"
#include "Components/WidgetComponent.h"
#include "multiplayerTEST/UI/Health/WorldSpaceHealthBar.h"
#include "multiplayerTEST/Components/PlayerComponents/PlayerAttacksComponent.h"
#include "multiplayerTEST/AnimInstances/PlayerAnimInstance.h"
#include "multiplayerTEST/Interactable/BonepileCore.h"
#include "multiplayerTEST/Characters/OceanCharacter/Passengers/ShipPlayerPassengerGhost.h"
#include "multiplayerTEST/GameplayInterfaces/Interactables/SpawnOverlapHandleInterface.h"

AmultiplayerTESTCharacter::AmultiplayerTESTCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMultiCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	static ConstructorHelpers::FClassFinder<UUserWidget> isLockOnWidgetClass(TEXT("/Game/UI/LockOnPoint"));

	if (isLockOnWidgetClass.Succeeded())
	{
		LockOnPointWidgetClass = isLockOnWidgetClass.Class;
	}

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBooom = CreateDefaultSubobject<UMySpringArmComponent>(TEXT("CameraaBooom"));
	CameraBooom->SetupAttachment(RootComponent);
	CameraBooom->TargetArmLength = 400.0f;
	CameraBooom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBooom, UMySpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>("HealthComp");
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>("StaminaComp");
	RollComponent = CreateDefaultSubobject<URollComponent>("RollComp");
	SprintComponent = CreateDefaultSubobject<USprintComponent>("SprintComp");
	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>("LockOnComp");
	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComp");
	HitDetectionComp = CreateDefaultSubobject<UHitDetectionComponent>("HitDetectionComp");
	ImpactComp = CreateDefaultSubobject<UImpactComponent>("ImpactComp");
	PlayerAttacksComponent = CreateDefaultSubobject<UPlayerAttacksComponent>("PlayerAttacksComp");

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = SprintComponent->GetWalkSpeed();
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	HelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelmetMesh"));
	HelmetMesh->SetupAttachment(GetMesh());

	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestMesh"));
	ChestMesh->SetupAttachment(GetMesh());

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh"));
	LegsMesh->SetupAttachment(GetMesh());

	WeaponRHMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshRH"));
	WeaponRHMesh->SetupAttachment(GetMesh(), "hand_r");

	WeaponLHMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshLH"));
	WeaponLHMesh->SetupAttachment(GetMesh(), "hand_l");

	HeartMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeartMesh"));
	HeartMesh->SetupAttachment(GetMesh(), "spine_05");

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

	HelmetMesh->SetCollisionProfileName(TEXT("NoCollision"));
	ChestMesh->SetCollisionProfileName(TEXT("NoCollision"));
	LegsMesh->SetCollisionProfileName(TEXT("NoCollision"));
	WeaponRHMesh->SetCollisionProfileName(TEXT("NoCollision"));
	WeaponLHMesh->SetCollisionProfileName(TEXT("NoCollision"));
	HeartMesh->SetCollisionProfileName(TEXT("NoCollision"));

	ChosenMaterial = 0;

	bSetUpFinished = false;
}

void AmultiplayerTESTCharacter::BeginPlay()
{
	bSetUpFinished = false;
	Super::BeginPlay();
	CustomController = Cast<AMyPlayerController>(Controller);
		
	if (CustomController) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(CustomController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	HealthComponent->ActorDied.AddDynamic(this, &AmultiplayerTESTCharacter::Die);

	GetCharacterMovement()->MaxWalkSpeed = SprintComponent->GetWalkSpeed();

	customAnimInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());

	SetUpCharacter();
	// get game state here
}

void AmultiplayerTESTCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AmultiplayerTESTCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &AmultiplayerTESTCharacter::StartMove);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AmultiplayerTESTCharacter::StopMove);


		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AmultiplayerTESTCharacter::Look);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AmultiplayerTESTCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AmultiplayerTESTCharacter::StopSprint);

		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &AmultiplayerTESTCharacter::Roll);

		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &AmultiplayerTESTCharacter::LockOn);

		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AmultiplayerTESTCharacter::HeavyAttack);
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AmultiplayerTESTCharacter::LightAttack);
		EnhancedInputComponent->BindAction(DoubleLightAttackAction, ETriggerEvent::Started, this, &AmultiplayerTESTCharacter::DoubleLightAttackOrBlock);
		EnhancedInputComponent->BindAction(DoubleLightAttackAction, ETriggerEvent::Completed, this, &AmultiplayerTESTCharacter::OnEndBlock);

		EnhancedInputComponent->BindAction(OpenEscMenuAction, ETriggerEvent::Started, this, &AmultiplayerTESTCharacter::OpenEscMenu);
		EnhancedInputComponent->BindAction(TwoHandAction, ETriggerEvent::Started, this, &AmultiplayerTESTCharacter::TryTwoHand);

	}
}

void AmultiplayerTESTCharacter::Move(const FInputActionValue& Value)
{
	MoveCharacter(Value.Get<FVector2D>(), this);
}

void AmultiplayerTESTCharacter::MoveCharacter(const FVector2D& Input, ACharacter* CharacterToMove)
{
	LastMovementVector = Input;
	LastMovementTime = GetWorld()->GetTimeSeconds(); // needed to not use old directions when rolling

	if (!CharacterToMove->Controller) {
		return;
	}

	if (bEndedFirstRollAttack)
	{
		TryInterruptRollAttack();
		bEndedFirstRollAttack = false;
	}

	const FRotator Rotation = CharacterToMove->Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	CharacterToMove->AddMovementInput(ForwardDirection, LastMovementVector.Y);
	CharacterToMove->AddMovementInput(RightDirection, LastMovementVector.X);

	if (LockOnComponent->GetIsLockedOn()) return;

	if (LastMovementVector.X < 0.05 && LastMovementVector.X >= 0 && FMath::Abs(LastMovementVector.Y) > 0.95) {
		CurrentCameraNudge = 0;
		return;
	}

	float TargetYaw = CharacterToMove->GetActorRotation().Yaw;
	float CurrentYaw = CharacterToMove->Controller->GetControlRotation().Yaw;
	float AngleDiff = FMath::FindDeltaAngleDegrees(CurrentYaw, TargetYaw);

	// Determine target nudge
	float TargetCameraNudgeScale = 1;
	if (OwnerTagManager && OwnerTagManager->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.UnrestrictiveEffects.Moving.Running")))) {
		TargetCameraNudgeScale = 1.5;
	}

	float MaxNudge = 0.28f * TargetCameraNudgeScale;   // at 90�
	float MinNudge = 0.07f * TargetCameraNudgeScale;   // at 0� or 180�

	// absolute angle difference
	float AbsAngle = FMath::Abs(AngleDiff); // 0..180

	// compute distance from 90-degree peak
	float DistanceFromPeak = FMath::Abs(AbsAngle - 90.f);   // 0 at peak, up to 90 at extremes

	// normalize to 0..1
	float Alpha = 1.f - FMath::Clamp(DistanceFromPeak / 90.f, 0.f, 1.f);

	// final magnitude
	float NudgeMagnitude = FMath::Lerp(MinNudge, MaxNudge, Alpha);

	// restore sign (left/right)
	float TargetCameraNudge = (AngleDiff > 0.f ? NudgeMagnitude : -NudgeMagnitude);

	// Decide interpolation speed
	float InterpSpeed = CameraNudgeSpeed; // normal speed

	// If direction changes, temporarily use a very high speed to "flip" fast
	if ((CurrentCameraNudge > 0.f && TargetCameraNudge < 0.f) ||
		(CurrentCameraNudge < 0.f && TargetCameraNudge > 0.f))
	{
		InterpSpeed = CameraNudgeSpeed * 10.f; // increase speed temporarily
	}

	// Interpolate smoothly toward the target nudge
	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	CurrentCameraNudge = FMath::FInterpTo(CurrentCameraNudge, TargetCameraNudge, DeltaSeconds, InterpSpeed);

	// Apply the nudge
	CharacterToMove->AddControllerYawInput(CurrentCameraNudge);
}

void AmultiplayerTESTCharacter::StartMoveFunc()
{
	if (GetLocalRole() != ROLE_Authority) {
		StartMoveServer();
	}
	SprintComponent->TrySprintFromMoving();
}

void AmultiplayerTESTCharacter::StartMove(const FInputActionValue& Value)
{
	StartMoveFunc();
}

void AmultiplayerTESTCharacter::StartMoveServer_Implementation()
{
	SprintComponent->TrySprintFromMoving();
}

void AmultiplayerTESTCharacter::StopMoveFunc() {
	if (GetLocalRole() != ROLE_Authority) {
		StopMoveServer();
	}
	CurrentCameraNudge = 0;
	if (OwnerTagManager) {
		OwnerTagManager->RemoveTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.UnrestrictiveEffects.Moving.Walking")));
	}
}

void AmultiplayerTESTCharacter::StopMove(const FInputActionValue& Value)
{	
	StopMoveFunc();
}

void AmultiplayerTESTCharacter::StopMoveServer_Implementation()
{
	if (OwnerTagManager) {
		OwnerTagManager->RemoveTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.UnrestrictiveEffects.Moving.Walking")));
	}
}

void AmultiplayerTESTCharacter::Look(const FInputActionValue& Value)
{
	LookCharacter(Value, this);
}

void AmultiplayerTESTCharacter::LookCharacter(const FInputActionValue& Value, ACharacter* CharacterWhereisCamera)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (CharacterWhereisCamera->Controller == nullptr)
	{
		return;
	}

	if (!LockOnComponent->GetIsLockedOn()) {
		CharacterWhereisCamera->AddControllerYawInput(LookAxisVector.X);
		CharacterWhereisCamera->AddControllerPitchInput(LookAxisVector.Y);
	}
	else if (LockOnComponent->GetCanChangeTargetAgain()) {
		LastXValue += LookAxisVector.X;
		LastYValue += LookAxisVector.Y;
	}
}

void AmultiplayerTESTCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AmultiplayerTESTCharacter, ChosenMaterial);
}

void AmultiplayerTESTCharacter::StartSprintFunc() {
	if (GetLocalRole() != ROLE_Authority) {
		StartSprintSeerver();
	}

	SprintComponent->TryStartSprint();

	SetDefaultMovementOrientation();

}

void AmultiplayerTESTCharacter::StopSprintFunc()
{
	if (GetLocalRole() != ROLE_Authority) {
		StopSprintServer();
	}
	
	SprintComponent->TryStopSprint();

	if (LockOnComponent->GetIsLockedOn()) {
		ChangeMovementOrientationForLockOn();
	}
}

void AmultiplayerTESTCharacter::StartSprint(const FInputActionValue& Value)
{
	StartSprintFunc();

	SetDefaultMovementOrientation();
}

void AmultiplayerTESTCharacter::StartSprintSeerver_Implementation()
{
	SprintComponent->TryStartSprint();

	SetDefaultMovementOrientation();
}

void AmultiplayerTESTCharacter::StopSprint(const FInputActionValue& Value)
{
	if (GetLocalRole() != ROLE_Authority) {
		StopSprintServer();
	}

	SprintComponent->TryStopSprint();

	if (LockOnComponent->GetIsLockedOn()) {
		ChangeMovementOrientationForLockOn();
	}
}

void AmultiplayerTESTCharacter::StopSprintServer_Implementation()
{
	SprintComponent->TryStopSprint();

	if (LockOnComponent->GetIsLockedOn()) {
		ChangeMovementOrientationForLockOn();
	}
}

void AmultiplayerTESTCharacter::TryRollFunc()
{
	const bool bMovementFresh = (GetWorld()->GetTimeSeconds() - LastMovementTime) <= 0.3f;

	FVector2D RollVector = bMovementFresh ? LastMovementVector : FVector2D(0.f, 1.f);

	if (RollComponent->TryRoll(RollVector.X, RollVector.Y))
	{
		SetSavedInputAction(
			[this, RollVector]() -> bool
			{
				return RollComponent->TryRoll(RollVector.X, RollVector.Y);
			}
		);
	}
}

void AmultiplayerTESTCharacter::Roll(const FInputActionValue& Value)
{
	TryRollFunc();
}


void AmultiplayerTESTCharacter::LockOn(const FInputActionValue& Value)
{
	LockOnComponent->TryLockOn(0, 0);
}

void AmultiplayerTESTCharacter::StartRagdoll()
{
	GetCapsuleComponent()->DestroyComponent();
	this->GetCharacterMovement()->DisableMovement();
	this->GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	this->GetMesh()->SetAllBodiesSimulatePhysics(true);
	
	if (!GetHealthBarWidget()) {
		return;
	}
	GetHealthBarWidget()->DestroyComponent();
}

// runs on both client and server through delegate binding
void AmultiplayerTESTCharacter::Die(AActor* DiedActor)
{
	AMultiplayerTestGameStateBase* OURGameState = Cast<AMultiplayerTestGameStateBase>(GetWorld()->GetGameState());
	if (!OURGameState || !customPlayerState) {
		UE_LOG(LogTemp, Error, TEXT("Game state or player state is invalid in ::Die inside %s"), *GetOwner()->GetName());
		return; 
	}

	if(UAnimInstance* MeshAnimInstance = GetMesh()->GetAnimInstance()){
		MeshAnimInstance->StopAllMontages(0);
	}

	OURGameState->OnPlayerCharacterDied(customPlayerState);

	if (StaminaComponent) { StaminaComponent->OnOwnerDeath(); }
	if (CustomController) { CustomController->OnOwnerDied(); }
	StartRagdoll();
	if (GetLocalRole() == ROLE_Authority) {

		AGameModeBase* GM = GetWorld()->GetAuthGameMode();
		if (AmultiplayerTESTGameMode* GameMode = Cast<AmultiplayerTESTGameMode>(GM)) {
			GameMode->Respawn(CustomController);
		}

		GetWorld()->GetTimerManager().SetTimer(DestoryHandle, this, &AmultiplayerTESTCharacter::CallDestroy, 10.0f, false);
	
	}
}

void AmultiplayerTESTCharacter::CallDestroy()
{
	if (GetLocalRole() == ROLE_Authority) {
		Destroy();
	}
}

void AmultiplayerTESTCharacter::CallRestartPlayer()
{
	//Destroy the Player.   
	Destroy();

	//Get the World and GameMode in the world to invoke its restart player function.
	if (UWorld* World = GetWorld())
	{
		if (AmultiplayerTESTGameMode* GameMode = Cast<AmultiplayerTESTGameMode>(World->GetAuthGameMode()))
		{
			GameMode->RestartPlayer(CustomController);
		}
	}
}

void AmultiplayerTESTCharacter::SetUpCharacter()
{
	CustomCharMoveComp = Cast<UMultiCharacterMovementComponent>(GetCharacterMovement());

	AMultiplayerTestGameStateBase* CustomGameState = Cast<AMultiplayerTestGameStateBase>(GetWorld()->GetGameState());

	customPlayerState = Cast<AMultiplayerTestPlayerState>(GetPlayerState());

	AMultiplayerTestGameStateBase* GameState = Cast<AMultiplayerTestGameStateBase>(GetWorld()->GetGameState());

	if (!customPlayerState || !CustomGameState || !GameState) {
		FTimerHandle TimerHandle_SetUpCharacter;

		GetWorld()->GetTimerManager().SetTimer(TimerHandle_SetUpCharacter, this, &AmultiplayerTESTCharacter::SetUpCharacter, 0.1f, false);

		return;
	}

	if (GetLocalRole() == ROLE_Authority || IsLocallyControlled()) {
		CustomController = Cast<AMyPlayerController>(GetController());

		if (!CustomController) {
			FTimerHandle TimerHandle_SetUpCharacter;

			GetWorld()->GetTimerManager().SetTimer(TimerHandle_SetUpCharacter, this, &AmultiplayerTESTCharacter::SetUpCharacter, 0.1f, false);

			return;
		}

		LockOnComponent->SetOwnerController(CustomController);

		if (IsLocallyControlled()) CustomController->CreateHUD();
	}

	OwnerTagManager = customPlayerState->GetTagManager();
	OwnerTagManager->SetUpInitialTags();
	PlayerAttacksComponent->OwnerTagManager = OwnerTagManager;

	CustomCharMoveComp->OwnerTagManagerComp = OwnerTagManager;

	SprintComponent->SetUpData(StaminaComponent, OwnerTagManager, CustomCharMoveComp);

	AbilitySystemComp->SetUpData(this, OwnerTagManager);

	OwnerInventoryComp = GetPlayerState()->GetComponentByClass<UInventoryComponent>();

	OwnerInventoryComp->AfterDeathSetUp(this);

	SetupEquippedItems();

	GameState->OnSomeoneSpawned(customPlayerState);
	bSetUpFinished = true;
	OverlapActorsAtSpawn();
}

void AmultiplayerTESTCharacter::SetupEquippedItems() {
	if (!OwnerInventoryComp || !OwnerInventoryComp->GetbIsInventoryInitolized()) { return; }

	EquipHelper(OwnerInventoryComp->SaveeHelmet, HelmetMesh, TEXT("HelmetMask"));
	EquipHelper(OwnerInventoryComp->SaveChest, ChestMesh, TEXT("ChestMask"));
	EquipHelper(OwnerInventoryComp->SaveLegs, LegsMesh, TEXT("LegsMask"));
	EquipHelper(OwnerInventoryComp->SaveHeartItem, HeartMesh);
	EquipHelper(OwnerInventoryComp->SaveWeaponRH1, WeaponRHMesh);
	EquipHelper(OwnerInventoryComp->SaveWeaponLH1, WeaponLHMesh, NAME_None, true);
}

void AmultiplayerTESTCharacter::OverlapActorsAtSpawn()
{
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
	
TArray<FString> AmultiplayerTESTCharacter::GetSocketNames() const
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

void AmultiplayerTESTCharacter::CallDrawCanRestAtBonfireWidget()
{
	if (!CustomController || !CustomController->CustomHUD || !OwnerTagManager) return;

	bWantToDrawCanRestAtBonepileUI = true;
	if (OwnerTagManager->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.RestrictiveEffects.Falling"))) ||
		OwnerTagManager->GetTags().HasTag(BonfireRestTag))
	{
		return;
	}

	CustomController->CustomHUD->DrawCanRestAtBonfireUI();
}

void AmultiplayerTESTCharacter::CallCloseCanRestAtBonfireWidget()
{
	if (!CustomController || !CustomController->CustomHUD) return;

	bWantToDrawCanRestAtBonepileUI = false;

	CustomController->CustomHUD->CloseCanRestAtBonfireUI();
}

void AmultiplayerTESTCharacter::TeleportActor(FVector Destination, FRotator NewRotation, bool DisableMovement = false)
{
	const bool bTeleported = TeleportTo(Destination, NewRotation, /*bSweep*/ false, /*bTeleportPhysics*/ true);
	
	if (DisableMovement) {
		GetCharacterMovement()->DisableMovement();
	}

	if (!bTeleported)
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportActor failed for %s to %s"), *GetName(), *Destination.ToString());
	}

	if (GetLocalRole() != ROLE_Authority)
	{
		TeleportActorServer(Destination, NewRotation);
	}
}

void AmultiplayerTESTCharacter::EquipHelper(UWearableItem* NewItem, 
	USkeletalMeshComponent* EquipSlot, 
	FName MaskParameterName, 
	bool bInvertRelativePosition)
{
	bool bIsWeapon = EquipSlot == WeaponRHMesh || EquipSlot == WeaponLHMesh;

	if(bIsWeapon){
		SetIsTwoHandFalse();
		PlayerAttacksComponent->ResetCombo();
	}

	EquipItemVisually(NewItem, EquipSlot, bInvertRelativePosition);
	UpdateMaskHelperFunction(NewItem, MaskParameterName);

	if (!bIsWeapon || !NewItem) { return; }

	UWeapon* IndeedWeapon = Cast<UWeapon>(NewItem);

	if (!IndeedWeapon) { // so its not take off since NewWeapon is not nullptr and still not weapon since cast failed
		UE_LOG(LogTemp, Error, TEXT("Equiped item into weapon slot when its not weapon %s"), *NewItem->GetName())
			return;
	}

	LHBlockImpactMontage = IndeedWeapon->GetBlockImpactMontage();
	LHBlockLoopSequence = IndeedWeapon->GetBlockLoopSequence();
	LHBlockStartSequence = IndeedWeapon->GetBlockStartSequence();
	LHBlockEndSequence = IndeedWeapon->GetBlockEndSequence();

	HitDetectionComp->GetNewWeaponData(IndeedWeapon, EquipSlot);
}

void AmultiplayerTESTCharacter::EquipItemVisually(UWearableItem* NewItem,
	USkeletalMeshComponent* EquipSlot,
	bool bInvertRelativePosition)
{
	if (!EquipSlot)
		return;

	if (!NewItem) {
		EquipSlot->SetSkinnedAssetAndUpdate(nullptr, true);
		return;
	}

	EquipSlot->SetSkinnedAssetAndUpdate(NewItem->SkeletMesh, true);

	FVector FinalLocation = NewItem->RelativeLocationOffset;
	FRotator FinalRotation = NewItem->RelativeRotationOffset;
	FVector ActualScale = NewItem->ScaleWhenEquipped;

	if (!bInvertRelativePosition)
	{
		EquipSlot->SetRelativeTransform(FTransform(FinalRotation.Quaternion(), FinalLocation, ActualScale));

		return;
	}

	FinalLocation *= -1;
	FinalRotation.Roll += 180;

	UWeapon* NewWeaponItem = Cast<UWeapon>(NewItem);

	if (!NewWeaponItem) return;

	if (NewWeaponItem->invertYawWhenEquipLH) {
		FinalRotation.Yaw += 180;
	}
	EquipSlot->SetRelativeTransform(FTransform(FinalRotation.Quaternion(), FinalLocation, ActualScale));
}

UWearableItem* AmultiplayerTESTCharacter::GetHelmet_Implementation()
{
	if (OwnerInventoryComp) {
		return OwnerInventoryComp->SaveeHelmet;
	}
	
	return nullptr;
}

UWearableItem* AmultiplayerTESTCharacter::GetChest_Implementation()
{
	if (OwnerInventoryComp) {
		return OwnerInventoryComp->SaveChest;
	}

	return nullptr;
}

UWearableItem* AmultiplayerTESTCharacter::GetLegs_Implementation()
{
	if (OwnerInventoryComp) {
		return OwnerInventoryComp->SaveLegs;
	}

	return nullptr;
}

UWearableItem* AmultiplayerTESTCharacter::GetWeaponRH1_Implementation()
{
	if (OwnerInventoryComp) {
		return OwnerInventoryComp->SaveWeaponRH1;
	}

	return nullptr;
}

UWearableItem* AmultiplayerTESTCharacter::GetWeaponLH1_Implementation()
{
	if (OwnerInventoryComp) {
		return OwnerInventoryComp->SaveWeaponLH1;
	}

	return nullptr;
}

bool AmultiplayerTESTCharacter::StartFight_Implementation(AActor* EncounterInitiator)
{
	if(CustomController && CustomController->OnFightStart(EncounterInitiator)) {
		if(!IsLocallyControlled()){
			StartFightClient();
		}

		if (customPlayerState) {
			customPlayerState->GetTagManager()->AddTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.InCombat")));
		}

		return true;
	}
	
	return false;
}

void AmultiplayerTESTCharacter::StartFightClient_Implementation()
{
	if (customPlayerState && customPlayerState->GetTagManager()) {
		customPlayerState->GetTagManager()->AddTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.InCombat")));
	}
}
bool AmultiplayerTESTCharacter::bCanStartFight(AActor* Enemy)const
{
	return !HealthComponent->GetIsDead();
}

UGameplayTagManagerComp* AmultiplayerTESTCharacter::GetTagManager_Implementation() const
{
	return OwnerTagManager;
}

bool AmultiplayerTESTCharacter::CanBeLockedOn_Implementation() const
{
	if (!HealthComponent) return false;

	return !HealthComponent->GetIsDead();
}

const TArray<FSocketInfo> AmultiplayerTESTCharacter::GetLockOnSocketNames_Implementation() const
{
	return LockOnSocketsInfo;
}

USkeletalMeshComponent* AmultiplayerTESTCharacter::GetLockOnTargetMesh_Implementation() const
{
	return GetMesh();
}

void AmultiplayerTESTCharacter::OnLockedOnByActor_Implementation(FName SelectedSocket)
{
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);

	GetLockOnPointWidget()->AttachToComponent(GetMesh(), AttachmentRules, SelectedSocket);

	GetLockOnPointWidget()->SetVisibility(true);

	UWidgetComponent* HealthWidgetAboveHead = GetHealthBarWidget();
	if (!HealthWidgetAboveHead) {
		return;
	}

	UWorldSpaceHealthBar* HealthbarWidget = Cast<UWorldSpaceHealthBar>(HealthWidgetAboveHead->GetWidget());
	if (HealthbarWidget) {
		HealthbarWidget->SetLockedOn(true);
	}
//	Cast<UWorldSpaceHealthBar>(GetHealthBarWidget()->GetWidget())->SetLockedOn(true);
}

void AmultiplayerTESTCharacter::StopLockOn_Implementation()
{
	UWidgetComponent* LockOnWidgetComp = GetLockOnPointWidget();

	if (!LockOnWidgetComp) {
		return;
	}

	LockOnWidgetComp->SetVisibility(false);

	UWidgetComponent* HealthBar = GetHealthBarWidget();
	if (!IsValid(HealthBar) || !IsValid(HealthBar->GetWidget())) {
		return;
	}
	Cast<UWorldSpaceHealthBar>(HealthBar->GetWidget())->SetLockedOn(false);
}

void AmultiplayerTESTCharacter::TeleportActorServer_Implementation(FVector Destination, FRotator NewRotation)
{
	TeleportActorMulticast(Destination, NewRotation);
}

void AmultiplayerTESTCharacter::TeleportActorMulticast_Implementation(FVector Destination, FRotator NewRotation)
{
	TeleportTo(Destination, NewRotation);
}

void AmultiplayerTESTCharacter::OnRep_ChosenMaterial()
{
	OnUpdateChosenMaterial();
}

void AmultiplayerTESTCharacter::SetChosenMaterial(int value)
{
	if (GetLocalRole() == ROLE_Authority) {
		ChosenMaterial = value;
		OnUpdateChosenMaterial();
	}
}

void AmultiplayerTESTCharacter::multiSetUpCharacterNameTag_Implementation(const FText& NewNametag)
{
	SetCharacterNametag(NewNametag);
}

void AmultiplayerTESTCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	CustomController = Cast<AMyPlayerController>(NewController);
}

void AmultiplayerTESTCharacter::Jump()
{
	if (!OwnerTagManager) { return; }

	if (OwnerTagManager->GetTags().HasAny(JumpBlockTags)) {
		return;
	}

	Super::Jump();	
}

void AmultiplayerTESTCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (bWantToDrawCanRestAtBonepileUI) {
		if (!CustomController || !CustomController->CustomHUD) return;

		CustomController->CustomHUD->DrawCanRestAtBonfireUI();
	}
}

void AmultiplayerTESTCharacter::ChangeMovementOrientationForLockOn()
{
	if (OwnerTagManager && !OwnerTagManager->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.UnrestrictiveEffects.Moving.Running")))) {
		ChangeMovementOrientationForLockOnServer();
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;

		if (PossessedGhost) {
			PossessedGhost->GetCharacterMovement()->bOrientRotationToMovement = false;
			PossessedGhost->GetCharacterMovement()->bUseControllerDesiredRotation = true;
		}
	}
}

void AmultiplayerTESTCharacter::ChangeMovementOrientationForLockOnServer_Implementation() {
	if (OwnerTagManager && !OwnerTagManager->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.UnrestrictiveEffects.Moving.Running")))) {
		ChangeMovementOrientationForLockOnMulticast();
	}
}
void AmultiplayerTESTCharacter::ChangeMovementOrientationForLockOnMulticast_Implementation() {
	if (IsAnyLocalControl()) return;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	if (PossessedGhost) {
		PossessedGhost->GetCharacterMovement()->bOrientRotationToMovement = false;
		PossessedGhost->GetCharacterMovement()->bUseControllerDesiredRotation = true;
	}
}

void AmultiplayerTESTCharacter::SetDefaultMovementOrientation()
{
	SetDefaultMovementOrientationServer();

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	if (PossessedGhost) {
		PossessedGhost->GetCharacterMovement()->bOrientRotationToMovement = true;
		PossessedGhost->GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}
}

void AmultiplayerTESTCharacter::SetDefaultMovementOrientationServer_Implementation()
{
	SetDefaultMovementOrientationMulticast();
}

void AmultiplayerTESTCharacter::SetDefaultMovementOrientationMulticast_Implementation()
{
	if (IsAnyLocalControl()) return;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	if (PossessedGhost) {
		PossessedGhost->GetCharacterMovement()->bOrientRotationToMovement = true;
		PossessedGhost->GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}
}

void AmultiplayerTESTCharacter::RotateCharacterFromDoubles(double x, double y)
{
	FRotator ControllerRotation = GetControlRotation();

	if (PossessedGhost) {
		ControllerRotation = PossessedGhost->GetControlRotation();
	}

	const FRotator YawRotation(0, ControllerRotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	FVector RollDirection = ForwardDirection * x + RightDirection * y;
	RollDirection.Normalize();

	FRotator TargetRotation = RollDirection.Rotation();

	if (!PossessedGhost) {
		SetActorRotation(TargetRotation);
		return;
	}
	PossessedGhost->SetActorRotation(TargetRotation);
}

void AmultiplayerTESTCharacter::TryLastInputAction_Implementation()
{
	if (SavedLastInputAction)
	{
		if (!SavedLastInputAction()) {
			SavedLastInputAction = nullptr;
		}
	}
}

void AmultiplayerTESTCharacter::SetSavedInputAction(TFunction<bool()> InAction)
{
	SavedLastInputAction = MoveTemp(InAction); // moveTemp is like assign but without copy lamda again

	// Reset the timer every time we set a new action
	GetWorld()->GetTimerManager().ClearTimer(SavedInputResetTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		SavedInputResetTimerHandle,
		this,
		&AmultiplayerTESTCharacter::ClearSavedInputAction,
		0.6f,      // delay in seconds
		false    
	);
}

void AmultiplayerTESTCharacter::ClearSavedInputAction()
{
	SavedLastInputAction = nullptr;
}

void AmultiplayerTESTCharacter::TryHeavyAttackFunc() {
	if (PlayerAttacksComponent->TryHeavyAttack()) {
		SetSavedInputAction(
			[this]() -> bool
			{
				return PlayerAttacksComponent->TryHeavyAttack();
			}
		);
	}
}


void AmultiplayerTESTCharacter::HeavyAttack(const FInputActionValue& Value)
{
	TryHeavyAttackFunc();
}

void AmultiplayerTESTCharacter::TryLightAttackFunc() {
	if (PlayerAttacksComponent->TryLightAttack()) {
		SetSavedInputAction(
			[this]() -> bool
			{
				return PlayerAttacksComponent->TryLightAttack();
			}
		);
	}
}

void AmultiplayerTESTCharacter::LightAttack(const FInputActionValue& Value)
{
	TryLightAttackFunc();
}

void AmultiplayerTESTCharacter::TryDoubleLightAttackOrBlockFunc() {
	if (PlayerAttacksComponent->TryDoubleLightAttackOrBlock()) {
		SetSavedInputAction(
			[this]() -> bool
			{
				return PlayerAttacksComponent->TryDoubleLightAttack();
			}
		);
	}
}

void AmultiplayerTESTCharacter::DoubleLightAttackOrBlock(const FInputActionValue& Value)
{
	TryDoubleLightAttackOrBlockFunc();
}

void AmultiplayerTESTCharacter::OpenEscMenuFunc() {
	if (!CustomController || !CustomController->CustomHUD) return;

	CustomController->CustomHUD->DrawEscMenu();
}

void AmultiplayerTESTCharacter::OpenEscMenu(const FInputActionValue& Value)
{
	OpenEscMenuFunc();
}

// function handles both starting two hand and stopping
void AmultiplayerTESTCharacter::TryTwoHandFunc()
{
	if (!Execute_GetWeaponRH1(this) || !Execute_GetWeaponRH1(this)->GetClass()->ImplementsInterface(UWeaponInfoInterface::StaticClass()))
	{
		return;
	}

	if (!OwnerTagManager || !customAnimInstance) return;


	// no two hand shields for now
	if (Execute_GetWeaponRH1(this)->ItemSubCategory == EItemSubCategory::Shield) return;

	customAnimInstance->WantToBlock = false;

	// flip flop if two hand weapon then
	// if not two hand then visible left weapon if two hand hide left weapon
	// then if we two hand then add tag that we two hand otherwise remove it

	customAnimInstance->TwoHandWeapon = !customAnimInstance->TwoHandWeapon;
	WeaponLHMesh->SetVisibility(!customAnimInstance->TwoHandWeapon, false);

	if (customAnimInstance->TwoHandWeapon) {
		OwnerTagManager->AddTag(TwoHandingWeaponTag);
	}
	else {
		OwnerTagManager->RemoveTag(TwoHandingWeaponTag);
	}
}

// func just to stop two hand
void AmultiplayerTESTCharacter::SetIsTwoHandFalse()
{
	if (!OwnerTagManager || !customAnimInstance) return;

	customAnimInstance->TwoHandWeapon = false;

	OwnerTagManager->RemoveTag(TwoHandingWeaponTag);

	WeaponLHMesh->SetVisibility(true, false);
}

void AmultiplayerTESTCharacter::TryTwoHand(const FInputActionValue& Value)
{
	TryTwoHandFunc();
}

void AmultiplayerTESTCharacter::TryInterruptRollAttack() {
	IWeaponInfoInterface* CurrentWeapon = Cast<IWeaponInfoInterface>(GetWeaponRH1_Implementation());
	if (!CurrentWeapon)
		return ;

	UAnimMontage* Montage = CurrentWeapon->GetFirstAttackAfterRoll().AttackAnimation;
	if (!Montage)
		return ;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return ;

	if (AnimInstance->Montage_IsPlaying(Montage))
	{
		AnimInstance->Montage_Stop(0.25f, Montage);
		return ; 
	}
	return ; 
}

void AmultiplayerTESTCharacter::UpdateMaskHelperFunction(UWearableItem* ItemToGetMaskFrom, FName MaskParameter) {
	if (!PlayerDynamicMaterial || MaskParameter == NAME_None) return;

	if (!ItemToGetMaskFrom || !ItemToGetMaskFrom->ItemMaskTexture) {
		PlayerDynamicMaterial->SetTextureParameterValue(MaskParameter, DefaultItemMask);

		return;
	}

	UTexture* MaskTex = ItemToGetMaskFrom->ItemMaskTexture;

	if (!MaskTex) {
		PlayerDynamicMaterial->SetTextureParameterValue(MaskParameter, DefaultItemMask);

		return;
	}

	PlayerDynamicMaterial->SetTextureParameterValue(MaskParameter, MaskTex);
}

void AmultiplayerTESTCharacter::UpdateAllMasks() {
	UpdateMaskHelperFunction(GetChest_Implementation(), TEXT("ChestMask"));
	UpdateMaskHelperFunction(GetHelmet_Implementation(), TEXT("HelmetMask"));
	UpdateMaskHelperFunction(GetLegs_Implementation(), TEXT("LegsMask"));
}

void AmultiplayerTESTCharacter::SetMaterial(UMaterialInterface* NewMaterial) {
	if (!NewMaterial || !GetMesh())
		return;

	PlayerDynamicMaterial = UMaterialInstanceDynamic::Create(NewMaterial, this);
	if (!PlayerDynamicMaterial)
		return;

	GetMesh()->SetMaterial(0, PlayerDynamicMaterial);

	UpdateAllMasks();
}

void AmultiplayerTESTCharacter::OnStartBonfireRest() {
	// stop movement, set up tags, hide meshes, stop other inputs
	if (!HasAuthority() || !OwnerTagManager) return;
	GetCharacterMovement()->DisableMovement();
	
	OwnerTagManager->AddTag(BonfireRestTag);

	if (!CustomController || !CustomController->CustomHUD) return;

	CustomController->CustomHUD->DrawBonfireWidget(LastOverlappedBonfireID);
}

void AmultiplayerTESTCharacter::OnStartBonfireRestFromJump()
{
	if (!GetMesh()) return;

	if (CurrentlyOverlappedBonfire) {
		CurrentlyOverlappedBonfire->TryToSaveBonpile();
	}

	if(customAnimInstance){
		customAnimInstance->bWantToRestAtBonfire = !customAnimInstance->bWantToRestAtBonfire;
	}

	OnStartBonfireRest();
}


void AmultiplayerTESTCharacter::OnStartLoopBonfireRest()
{
	HelmetMesh->SetVisibility(false);
	ChestMesh->SetVisibility(false);
	LegsMesh->SetVisibility(false);
	WeaponRHMesh->SetVisibility(false);
	WeaponLHMesh->SetVisibility(false);
	HeartMesh->SetVisibility(false);
	GetMesh()->SetVisibility(false);
}

void AmultiplayerTESTCharacter::OnStopBonfireRestStarted() {
	if (!HasAuthority() || !OwnerTagManager) return;

	HelmetMesh->SetVisibility(true);
	ChestMesh->SetVisibility(true);
	LegsMesh->SetVisibility(true);
	WeaponRHMesh->SetVisibility(true);
	if (OwnerTagManager && !OwnerTagManager->GetTags().HasTag(TwoHandingWeaponTag)) {
		WeaponLHMesh->SetVisibility(true);
	}
	HeartMesh->SetVisibility(true);
	GetMesh()->SetVisibility(true);

	if (!CustomController || !CustomController->CustomHUD) return;

	CustomController->CustomHUD->CloseBonfireWidget();
}

void AmultiplayerTESTCharacter::OnStopBonfireRestEnded() {
	if (!HasAuthority() || !OwnerTagManager) return;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	if (MoveComp)
	{
		MoveComp->bJustTeleported = true;
		MoveComp->bForceNextFloorCheck = true;
		MoveComp->SetMovementMode(MOVE_Walking);

	}
	OwnerTagManager->RemoveTag(BonfireRestTag);

}

// used for bonfire
void AmultiplayerTESTCharacter::ResetCharacter() { 
	// reset health, UI, targets
	HealthComponent->ResetHealthComponent();
	ImpactComp->ResetImpactComponent();
	
	if (CustomController) {
		CustomController->ResetController();
		CustomController->ResetHUDAfterBonfireRest();
	}
}

bool AmultiplayerTESTCharacter::IsAnyLocalControl() const
{
	if (CustomController && CustomController->IsLocalController())
	{
		return true;
	}

	return false;
}
