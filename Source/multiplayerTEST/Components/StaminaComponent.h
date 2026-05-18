// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/AttributeComponentInfo.h"
#include "GameplayTagContainer.h"
#include "StaminaComponent.generated.h"

class UGameplayTagManagerComp;

DECLARE_DELEGATE_OneParam(FStaminaUpdateDelegate, float);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStaminaReachedValue);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIPLAYERTEST_API UStaminaComponent : public UActorComponent, public IAttributeComponentInfo
{
	GENERATED_BODY()

public:
	UStaminaComponent();

	void StartRegenStamina();
	void StartConsumeStamina();

	float AddStamina(float Value);
	void ModifyMaxStamina(float Value);

	UFUNCTION(BlueprintCallable)
	float GetCurrentStamina() const { return CurrentStamina; }
	UFUNCTION(BlueprintCallable)
	float GetMaxStamina() const { return MaxStamina; }

	void OnOwnerDeath();
protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

public:

	FStaminaUpdateDelegate OnStaminaUpdate;

	UPROPERTY(BlueprintAssignable)
	FStaminaReachedValue OnStaminaDepletedByTimer;

	UPROPERTY(BlueprintAssignable)
	FStaminaReachedValue OnStaminaDepletedByEvent;

	UPROPERTY(BlueprintAssignable)
	FStaminaReachedValue OnReachedMaxStaminaByTimer;

	UPROPERTY(BlueprintAssignable)
	FStaminaReachedValue OnReachedMaxStaminaByEvent;

	float GetAttributeValue() const override { return CurrentStamina; }

	void AddToAttribute(float ValueToAdd)override;
protected:

	void RegenerateStamina();

	void ConsumeStamina();
	UFUNCTION()
	void StartExhaustion();
	void StopExhaustion();

	void SetUpTagManager();
protected:
	UPROPERTY(EditAnywhere)
	float RecoveryDelayAfterExhaustion;

	UPROPERTY()
	float CurrentStamina;

	UPROPERTY()
	float MaxStamina;

	UPROPERTY()
	float StaminaRegenPerSecond;
	
	UPROPERTY()
	float StaminaConsumptionPerSecond;

	const float RegenInterval = 0.05f;

	bool isExhausted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer RegenBlockingTags;
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer StaminaRegenReduceTags;

	UGameplayTagManagerComp* OwnerTagManager;
protected:
	FTimerHandle RegenTimerHandle;
	FTimerHandle ConsumeTimerHandle;
	FTimerHandle ExhaustionTimerHandle;
};

