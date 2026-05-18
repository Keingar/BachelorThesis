
#include "AbilitySystemComponent.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/AbilitySystem/CoreAbility.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "multiplayerTEST/GameplayInterfaces/PlayerInformationInterface.h"

UAbilitySystemComponent::UAbilitySystemComponent()
{

}


void UAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UAbilitySystemComponent::SetUpData(APawn* NewAvatarActor, UGameplayTagManagerComp* NewOwnerTagManager)
{
    AvatarActor = NewAvatarActor;
    OwnerTagManager = NewOwnerTagManager;
}


void UAbilitySystemComponent::AddAbility(UCoreAbility* NewAbility)
{
    if (!NewAbility) return;
	Abilities.Add(NewAbility);
    OwnerTagManager->AddTags(NewAbility->GetTagsGrantedToOwner());
}

void UAbilitySystemComponent::RemoveAbility(UCoreAbility* AbilityToRemove)
{
    if (!AbilityToRemove) {
        return; 
    }

    for (const FGameplayTag& Tag : AbilityToRemove->GetTagsGrantedToOwner())
    {
        bool bTagStillUsed = false;

        for (UCoreAbility* ActiveAbility : Abilities)
        {
            if (ActiveAbility == AbilityToRemove) continue;

            if (ActiveAbility->GetTagsGrantedToOwner().HasTagExact(Tag))
            {
                bTagStillUsed = true;
                break;
            }
        }

        if (!bTagStillUsed)
        {
            // No other ability grants this tag — safe to remove it from the owner
            OwnerTagManager->RemoveTag(Tag); 
        }
    }

    if (IPlayerInformationInterface* OwnerInfo = Cast<IPlayerInformationInterface>(AvatarActor)) {
        OwnerInfo->TryLastInputAction_Implementation();
    }

    Abilities.Remove(AbilityToRemove);
}

bool UAbilitySystemComponent::DoesAnotherAbilityHaveTag(UCoreAbility* AbilityWhereTagIgnore, FGameplayTag TagToCheck)
{
    if (!AbilityWhereTagIgnore) {
        return false;
    }

    for (UCoreAbility* ActiveAbility : Abilities)
    {
        if (ActiveAbility == AbilityWhereTagIgnore) continue;

        if (ActiveAbility->GetTagsGrantedToOwner().HasTagExact(TagToCheck))
        {
            return true;
        }
    }

    return false;
}
