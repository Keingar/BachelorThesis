// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayTagManagerComp.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTEST_API UGameplayTagManagerComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGameplayTagManagerComp();

	UFUNCTION(BlueprintCallable, Category = "GameplayTags")
	FGameplayTagContainer& GetTags() { return PlayerTags; }

	UFUNCTION(BlueprintCallable, Category = "GameplayTags")
	void AddTag(FGameplayTag TagToAdd);
	UFUNCTION(BlueprintCallable, Category = "GameplayTags")
	void RemoveTag(FGameplayTag TagToRemove);
	UFUNCTION(BlueprintCallable, Category = "GameplayTags")
	void AddTags(FGameplayTagContainer TagsToAdd);
	UFUNCTION(BlueprintCallable, Category = "GameplayTags")
	void RemoveTags(FGameplayTagContainer TagsToRemove);

	void SetUpInitialTags();

	UFUNCTION(BlueprintCallable)
	FGameplayTagContainer& GetOwnerEnemiesFactionTags() { return OwnerEnemiesFactionTags; }
	UFUNCTION(BlueprintCallable)
	const FGameplayTag& GetOwnerFactionTag() { return OwnerFactionTag; }

	UFUNCTION(BlueprintCallable, Category = "Faction")
	void SetOwnerFactionTag(FGameplayTag NewOwnerFactionTag);

	UFUNCTION(BlueprintCallable, Category = "Faction")
	void SetOwnerEnemiesFactionTags(FGameplayTagContainer NewOwnerEnemiesFactionTags);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:	
	virtual void BeginPlay() override;

	UPROPERTY(Replicated)
	FGameplayTagContainer PlayerTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
	FGameplayTagContainer DefaultAliveTags;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
	FGameplayTagContainer DefaultDeadTags;

	UPROPERTY(EditDefaultsOnly, Category = "Faction")
	FGameplayTagContainer OwnerEnemiesFactionTags;

	UPROPERTY(EditDefaultsOnly, Category = "Faction")
	FGameplayTag OwnerFactionTag;

	FGameplayTag isDeadTag; // just to access it fast


protected:
	UFUNCTION()
	void ResetAfterDeath(AActor* DeadActor);
	
	UFUNCTION(BlueprintCallable, Category = "GameplayTags")
	void OverrideTags(FGameplayTagContainer TagsToSet);
};
