// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/AttributeComponentInfo.h"
#include "multiplayerTEST/CustomStructs/DamageInfo.h"
#include "multiplayerTEST/CustomStructs/BlockInfo.h"
#include "HealthComponent.generated.h"

class UGameplayTagManagerComp;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHealthDispatcher, float, Health, float, MaxHeaalth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStatusDispatcher, AActor*, OwnerActor);
DECLARE_MULTICAST_DELEGATE_TwoParams(FHealthChangedLeftOverr, float Healthhh, float MaxHealthhh); // health here can be lower than 0

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIPLAYERTEST_API UHealthComponent : public UActorComponent, public IAttributeComponentInfo
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHealthComponent();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable) // blueprint available for reset interface for some objects like mast
	void ResetHealthComponent();

	void SetUpHealthComponent();

	UFUNCTION(BlueprintCallable, Category="Health")
	const bool GetIsDead() const { return isDead; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetIsDead(bool newVal);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const { return health; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const { return maxHealth; }

	// returns true if actor died because of this damage
	UFUNCTION(BlueprintCallable, Category = "Health")
	bool TakeDamage(FDamageInfo TakenDamageInfo, FBlockInfo ImpactedShieldBlockInfo = FBlockInfo());
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float HealAmount);
protected:
	bool isDead;

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	int32 maxHealth;
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	int32 health;

	UGameplayTagManagerComp* OwnerTagComponent;

public:
	UPROPERTY(BlueprintAssignable)
	FHealthDispatcher HealthChanged;

	UPROPERTY(BlueprintAssignable)
	FStatusDispatcher ActorDied;

	FHealthChangedLeftOverr OnHealthChangedLeftOver; // just to bind health UI

	float GetAttributeValue() const override { return health; }
	void AddToAttribute(float ValueToAdd) override;

private:
	UFUNCTION()
	void OnRepHealth();

	UPROPERTY(ReplicatedUsing = OnRepHealth)
	int32 RepcurrentHealth;
	UPROPERTY(ReplicatedUsing = OnRepHealth)
	int32 RepMaxHealth;

	UFUNCTION(Server, Reliable)
	void ServerHeal(float HealAmount);
};
