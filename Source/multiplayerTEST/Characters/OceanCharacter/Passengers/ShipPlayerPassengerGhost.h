#pragma once

#include "CoreMinimal.h"
#include "ShipPassangerGhostCore.h"
#include "InputActionValue.h"
#include "ShipPlayerPassengerGhost.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AmultiplayerTESTCharacter;
class UMySpringArmComponent;

UCLASS()
class MULTIPLAYERTEST_API AShipPlayerPassengerGhost : public AShipPassangerGhostCore
{
	GENERATED_BODY()

public:
	AShipPlayerPassengerGhost(const FObjectInitializer& ObjectInitializer);

	void AttachCameraToActor(AActor* TargetActor);

	void TryAddMappingContext();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ship")
	AmultiplayerTESTCharacter* RealActorPlayer;

	void CopySettingsFromActor(ACharacter* RealCharacter) override;

protected:

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;

	void Move(const FInputActionValue& Value);
	void StartMove(const FInputActionValue& Value);
	void StopMove(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);
	
	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);

	void OpenEscMenu(const FInputActionValue& Value);

	void TryTwoHand(const FInputActionValue& Value);

	void LightAttack(const FInputActionValue& Value);

	void HeavyAttack(const FInputActionValue& Value);

	void DoubleLightAttackOrBlock(const FInputActionValue& Value);
	void TryEndBlock(const FInputActionValue& Value);

	void Roll(const FInputActionValue& Value);
	void LockOn(const FInputActionValue& Value);


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UMySpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

protected: // input stuff
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* OpenEscMenuAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* TwoHandAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* LightAttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* HeavyAttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* DoubleLightAttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* RollAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* LockOnAction;
};