  // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "RollComponent.generated.h"

class IAttributeComponentInfo;
class UGameplayTagManagerComp;
class UCoreAbility;
class URollBullshitData;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIPLAYERTEST_API URollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URollComponent();

	// return true if should be used for LastInputAction
	bool TryRoll(double XAxis, double YAxis);

protected:
	virtual void BeginPlay() override;

	void SetUpTagManager();
protected:
	TScriptInterface<IAttributeComponentInfo> StaminaComponent;
	UGameplayTagManagerComp* TagManager;


	UPROPERTY(EditAnywhere)
	float StaminaRequiredForRoll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UCoreAbility> RollAbilityClass;

	UFUNCTION(Server, Reliable)
	void TryRollAbilityOnServer(double XAxis, double YAxis);
private:
	UPROPERTY()
	URollBullshitData* BullshitData;
};
