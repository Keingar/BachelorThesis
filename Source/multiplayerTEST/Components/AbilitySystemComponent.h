

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemComponent.generated.h"

class UCoreAbility;
class UGameplayTagManagerComp;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIPLAYERTEST_API UAbilitySystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilitySystemComponent();

	UPROPERTY()
	TArray<UCoreAbility*> Abilities; // abilities that are currently active

public:
	void AddAbility(UCoreAbility* NewAbility);
	void RemoveAbility(UCoreAbility* AbilityToRemove);

	UFUNCTION(BlueprintCallable)
	bool DoesAnotherAbilityHaveTag(UCoreAbility* AbilityWhereTagIgnore, FGameplayTag TagToCheck);

	void SetUpData(APawn* NewAvatarActor, UGameplayTagManagerComp* NewOwnerTagManager);

protected:
	virtual void BeginPlay() override;

protected:

	UGameplayTagManagerComp* OwnerTagManager;

	APawn* AvatarActor;
};
