// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CoreAbility.generated.h"

class UGameplayTagManagerComp;

UCLASS(Blueprintable)
class MULTIPLAYERTEST_API UCoreAbility : public UObject
{
	GENERATED_BODY()

public:
	UCoreAbility();
	~UCoreAbility();

public:
	UFUNCTION(BlueprintCallable)
	static bool TryUseAbilityStatic(TSubclassOf<class UCoreAbility> AbilityClass,
		float NewCost,
		TScriptInterface<class IAttributeComponentInfo> AttributeComponentApplyCostTo,
		AActor* AvatarActorr,
		UGameplayTagManagerComp* TagsCompOfOwner,
		UObject* AdditionalDataa);
		
	static bool CanUseAbilityStatic(TSubclassOf<class UCoreAbility> AbilityClass, 
		TScriptInterface<class IAttributeComponentInfo> AttrubuteComponentApplyCostTo,
		class UGameplayTagManagerComp* TagsCompOfOwner, 
		UObject* AdditionalDataa);


	FGameplayTagContainer GetTagsGrantedToOwner() const { return TagsGrantedToOwner; }

protected:

	void SetUpAbility(float NewCost, class IAttributeComponentInfo* AttrubuteComponentApplyCostTo, AActor* AvatarActorr, class UGameplayTagManagerComp* TagsCompOfOwner, UObject* AdditionalDataa);

	UFUNCTION(BlueprintCallable)
	virtual bool CommitAbility();

	bool TryUseAbility(float NewCost,
		TScriptInterface<IAttributeComponentInfo> AttributeComponentApplyCostTo,
		AActor* AvatarActorr,
		UGameplayTagManagerComp* TagsCompOfOwner,
		UObject* AdditionalData);

	bool CanUseAbility();
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability")
	void UseAbility(); 

	UFUNCTION(BlueprintCallable, Category = "Ability")
	void EndAbility();

	void ApplyTagsToOwner();
	void RemoveTagsFromOwner();
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	bool isUsingStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FGameplayTagContainer AbilityTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FGameplayTagContainer TagsGrantedToOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FGameplayTagContainer ActivationBlockedTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FGameplayTagContainer ActivationRequiredTags;

	// if owner has any tag from group then he must have all tags from that group
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	TArray<FGameplayTagContainer> RequiredGroupsOftags;

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	AActor* AvatarActor; // physical actor for animations 

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	UGameplayTagManagerComp* TagManager;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	float Cost;

	class IAttributeComponentInfo* AttributeComponent;

	UPROPERTY(BlueprintReadOnly)
	UObject* AdditionalData;
private:
	virtual void BeginDestroy() override;
};
