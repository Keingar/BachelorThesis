// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "multiplayerTEST/GameplayInterfaces/EnvironmentInteractInterface.h"
#include "multiplayerTEST/GameplayInterfaces/EquipSystemInterface.h"
#include "multiplayerTEST/GameplayInterfaces/PlayerInformationInterface.h"
#include "multiplayerTEST/GameplayInterfaces/CombatInterface.h"
#include "GameplayTagContainer.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/TagManagerOwner.h"
#include "multiplayerTEST/GameplayInterfaces/LockOnInterface.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/GhostAnimInstanceSyncInterface.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/GetOwningMainMesh.h"
#include "multiplayerTESTCharacter.generated.h"

class UMySpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UHealthComponent;
class UStaminaComponent;
class URollComponent;
class USprintComponent;
class USkeletalMeshComponent;
class ULockOnComponent;
class UAbilitySystemComponent;
class UHitDetectionComponent;
class UImpactComponent;
class UPlayerAttacksComponent;
class UWidgetComponent;
class UGameplayTagManagerComp;
class UMultiCharacterMovementComponent;
class UInventoryComponent;
class AMyPlayerController;
class AMultiplayerTestPlayerState;
class ABonepileCore;
class AShipPlayerPassengerGhost;
class UPlayerAnimInstance;

UCLASS(config = Game)
class AmultiplayerTESTCharacter : public ACharacter, 
	public IEnvironmentInteractInterface,
	public IEquipSystemInterface,
	public IPlayerInformationInterface,
	public ICombatInterface,
	public ITagManagerOwner,
	public ILockOnInterface,
	public IGhostAnimInstanceSyncInterface,
	public IGetOwningMainMesh
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UMySpringArmComponent* CameraBooom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RollAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LockOnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HeavyAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LightAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DoubleLightAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* OpenEscMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TwoHandAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true"))
	UStaminaComponent* StaminaComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true"))
	URollComponent* RollComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true"))
	USprintComponent* SprintComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* HelmetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ChestMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* LegsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* HeartMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponRHMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemSlots", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponLHMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn", meta = (AllowPrivateAccess = "true"))
	ULockOnComponent* LockOnComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = "true"))
	UHitDetectionComponent* HitDetectionComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = "true"))
	UImpactComponent* ImpactComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = "true"))
	UPlayerAttacksComponent* PlayerAttacksComponent;

public:
	AmultiplayerTESTCharacter(const FObjectInitializer& ObjectInitializer);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE USprintComponent* GetSprintComponent() const { return SprintComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UImpactComponent* GetImpactComponent() const { return ImpactComp; }

	UFUNCTION(BlueprintPure, Category = "SaveSystem")
	FORCEINLINE int GetChosenMaterial() const { return ChosenMaterial; }

	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void SetChosenMaterial(int value);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Character")
	void SetCharacterNametag(const FText& NewNametag);

	UFUNCTION(Server, Reliable)
	void multiSetUpCharacterNameTag(const FText& NewNametag);

	void SetUpCharacter();
	void SetupEquippedItems();
	
	void OverlapActorsAtSpawn();

	virtual void PossessedBy(AController* NewController) override;

	virtual void Jump() override;
	void Landed(const FHitResult& Hit) override;


	float GetLastXValue() const { return LastXValue; }
	float GetLastYvalue() const { return LastYValue; }

	void SetLastXValue(float NewValue) { LastXValue = NewValue; }
	void SetLastYValue(float NewValue) { LastYValue = NewValue; }

	void ChangeMovementOrientationForLockOn();
	void SetDefaultMovementOrientation();

	UFUNCTION(BlueprintCallable)
	void RotateCharacterFromDoubles(double x, double y); // used for roll ability

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	UWidgetComponent* GetLockOnPointWidget();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	UWidgetComponent* GetHealthBarWidget();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* LHBlockImpactMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* LHBlockLoopSequence;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* LHBlockStartSequence;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* LHBlockEndSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* RHBlockImpactMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* RHBlockLoopSequence;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* RHBlockStartSequence;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* RHBlockEndSequence;

	UFUNCTION(BlueprintImplementableEvent)
	void OnStartBlock();

	// id is for UI
	void OnStartBonfireRest();

	UFUNCTION(BlueprintCallable)
	void OnStartLoopBonfireRest();

	// for begging and end of animation
	UFUNCTION(BlueprintCallable)
	void OnStartBonfireRestFromJump();
	UFUNCTION(BlueprintCallable)
	void OnStopBonfireRestStarted();
	UFUNCTION(BlueprintCallable)
	void OnStopBonfireRestEnded();

	void ResetCharacter();

	UPROPERTY()
	ABonepileCore* CurrentlyOverlappedBonfire;

	UPROPERTY(BlueprintReadOnly)
	ACharacter* PossessedGhost; // valid only on ship

	// needed to not copy entire anim instance of real character  
	// since anim instance is neeeded only to copy montages root motion
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAnimInstance> GhostAnimInstanceSync;

	bool bSetUpFinished = false;

	FName LastOverlappedBonfireID;

	void CallDrawCanRestAtBonfireWidget();
	void CallCloseCanRestAtBonfireWidget();
	bool bWantToDrawCanRestAtBonepileUI = false;
public:
	// public interfaces
	UFUNCTION()
	virtual void TeleportActor(FVector Destination, FRotator NewRotation, bool DisableMovement) override;

	virtual USkeletalMeshComponent* GetHelmetMesh() const override{ return HelmetMesh; };
	virtual USkeletalMeshComponent* GetChestMesh() const override{ return ChestMesh; };
	virtual USkeletalMeshComponent* GetLegsMesh() const override{ return LegsMesh; };
	virtual USkeletalMeshComponent* GetWeaponRH1_Mesh() const override{ return WeaponRHMesh; };
	virtual USkeletalMeshComponent* GetWeaponLH1_Mesh() const override{ return WeaponLHMesh; };
	virtual USkeletalMeshComponent* GetHeartMesh() const override{ return HeartMesh; };

	virtual void EquipHelper(
		UWearableItem* NewItem,
		USkeletalMeshComponent* EquipSlot,
		FName MaskParameterName = NAME_None,
		bool InvertRelativePosition = false
	) override;

	virtual UWearableItem* GetHelmet_Implementation()override;
	virtual UWearableItem* GetChest_Implementation()override;
	virtual UWearableItem* GetLegs_Implementation()override;
	virtual UWearableItem* GetWeaponRH1_Implementation()override;
	virtual UWearableItem* GetWeaponLH1_Implementation()override;

	virtual bool StartFight_Implementation(AActor* EncounterInitiator) override;
	virtual bool bCanStartFight(AActor* Enemy) const override;

	virtual UGameplayTagManagerComp* GetTagManager_Implementation() const override;

	// lock on interface
	bool CanBeLockedOn_Implementation() const override;

	virtual const TArray<FSocketInfo> GetLockOnSocketNames_Implementation() const override;

	virtual USkeletalMeshComponent* GetLockOnTargetMesh_Implementation() const override;

	virtual void OnLockedOnByActor_Implementation(FName SelectedSocket) override;

	virtual void StopLockOn_Implementation() override;

	virtual TArray<FLockOnCameraParams> GetLockOnCameraParams() override { return LockOnCameraParams; }

	virtual void TryLastInputAction_Implementation() override;

	virtual void SetbEndedFirstRollAttackToTrue_Implementation() override { bEndedFirstRollAttack = true; }

	virtual TSubclassOf<UAnimInstance> GetGhostAnimInstanceClass() const override { return GhostAnimInstanceSync; }
	virtual ACharacter* GetPossessedGhostCharacter_Implementation() const override { return PossessedGhost; }

	virtual UMeshComponent* GetOwningMainMesh_Implementation() const override { return GetMesh(); }

	// used for moving when ghost is controlling movement
	void MoveCharacter(const FVector2D& Value, ACharacter* CharacterToMove);
	void LookCharacter(const FInputActionValue& Value, ACharacter* CharacterWhereisCamera);
	void StartSprintFunc();
	void StopSprintFunc();
	void StartMoveFunc();
	void StopMoveFunc();
	void OpenEscMenuFunc();

	void TryTwoHandFunc();
	void TryLightAttackFunc();
	void TryHeavyAttackFunc();
	void TryDoubleLightAttackOrBlockFunc();
	void TryRollFunc();
	void LockOn(const FInputActionValue& Value);


	UFUNCTION(BlueprintImplementableEvent)
	void OnEndBlock(const FInputActionValue& Value);

	bool IsAnyLocalControl() const;

// input functions
protected:

	void Move(const FInputActionValue& Value);

	void StartMove(const FInputActionValue& Value);
	void StopMove(const FInputActionValue& Value);


	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);

	void Roll(const FInputActionValue& Value);

	void HeavyAttack(const FInputActionValue& Value);
	void LightAttack(const FInputActionValue& Value);
	void DoubleLightAttackOrBlock(const FInputActionValue& Value);

	void OpenEscMenu(const FInputActionValue& Value);
	void TryTwoHand(const FInputActionValue& Value);


protected:
	UPROPERTY(ReplicatedUsing = OnRep_ChosenMaterial)
	int ChosenMaterial;

	float CurrentCameraNudge = 0.f; // current applied camera nudge
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraNudgeSpeed = 5.f;   // units per second, tweak to feel right

	UFUNCTION()
	void OnRep_ChosenMaterial();

	UFUNCTION(BlueprintImplementableEvent, Category = "SaveSystem")
	void OnUpdateChosenMaterial();

	///////////////////////////////////////////////////////
	UFUNCTION(Server, Reliable)
	void StartSprintSeerver();
	UFUNCTION(Server, Reliable)
	void StopSprintServer();
	UFUNCTION(Server, Reliable)
	void StartMoveServer();
	UFUNCTION(Server, Reliable)
	void StopMoveServer();

	UFUNCTION(Client, Reliable)
	void StartFightClient();

	UFUNCTION(Server, Reliable)
	void ChangeMovementOrientationForLockOnServer();
	UFUNCTION(NetMulticast, Reliable)
	void ChangeMovementOrientationForLockOnMulticast();

	UFUNCTION(Server, Reliable)
	void SetDefaultMovementOrientationServer();
	UFUNCTION(NetMulticast, Reliable)
	void SetDefaultMovementOrientationMulticast();

	void SetIsTwoHandFalse();

	TFunction<bool()> SavedLastInputAction;
	void SetSavedInputAction(TFunction<bool()> InAction);
	FTimerHandle SavedInputResetTimerHandle;
	void ClearSavedInputAction();

	void UpdateAllMasks();
	void UpdateMaskHelperFunction(UWearableItem* ItemToGetMaskFrom, FName MaskParameter = FName());
	UFUNCTION(BlueprintCallable)
	void SetMaterial(UMaterialInterface* NewMaterial);

	UPROPERTY()
	UMaterialInstanceDynamic* PlayerDynamicMaterial = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UTexture* DefaultItemMask;
	/////////////////////////////////////////////////////

	void EquipItemVisually(UWearableItem* NewItem,
		USkeletalMeshComponent* EquipSlot,
		bool bInvertRelativePosition
	);

protected:
	// default constant values

	float LastXValue;
	float LastYValue;

	float LastMovementTime = 0.f; // used to not roll in direction that was moving in in the past 

	FVector2D LastMovementVector;

	bool bEndedFirstRollAttack = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	TArray<FSocketInfo> LockOnSocketsInfo;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UUserWidget> LockOnPointWidgetClass;

	UPROPERTY()
	UGameplayTagManagerComp* OwnerTagManager;
	UPROPERTY()
	UMultiCharacterMovementComponent* CustomCharMoveComp;
	UPROPERTY()
	UInventoryComponent* OwnerInventoryComp;

	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer JumpBlockTags;

	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag BonfireRestTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag TwoHandingWeaponTag;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	TArray<FLockOnCameraParams> LockOnCameraParams;

	UPROPERTY()
	AMyPlayerController* CustomController;
	UPROPERTY()
	UPlayerAnimInstance* customAnimInstance;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	void CallRestartPlayer();

	void StartRagdoll();

	UFUNCTION()
	void Die(AActor* DiedActor);

	FTimerHandle DestoryHandle;

	void CallDestroy();


	UFUNCTION(Server, Reliable)
	void TeleportActorServer(FVector Destination, FRotator NewRotation);

	UFUNCTION(NetMulticast, Reliable)
	void TeleportActorMulticast(FVector Destination, FRotator NewRotation);

	AMultiplayerTestPlayerState* customPlayerState;

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	TArray<FString> GetSocketNames() const;

	void TryInterruptRollAttack();

public:
	FORCEINLINE UMySpringArmComponent* GetCameraBoom() const { return CameraBooom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintPure, Category = "Input")
	FORCEINLINE UInputMappingContext* GetDefaultMappingContextAsset() const { return DefaultMappingContext; }

	UFUNCTION(BlueprintPure, Category = "Input")
	FORCEINLINE UInputAction* GetMoveInputActionAsset() const { return MoveAction; }

	UFUNCTION(BlueprintPure, Category = "Input")
	FORCEINLINE UInputAction* GetLookInputActionAsset() const { return LookAction; }
};

