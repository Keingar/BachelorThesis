#include "HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/Characters/Enemies/EnemyCore.h"
#include "GameFramework/PlayerState.h"

UHealthComponent::UHealthComponent()
{
	SetIsReplicatedByDefault(true);
	RepMaxHealth = 100;
	RepcurrentHealth = 100;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	SetUpHealthComponent();
}

bool UHealthComponent::TakeDamage(FDamageInfo TakenDamageInfo, FBlockInfo ImpactedShieldBlockInfo)
{
	//if (OwnerTagComponent && OwnerTagComponent->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.Buffs.Immune")))) {
	//	return;
	//}

	float Damage = (
		TakenDamageInfo.PhysDamage * (1.f - ImpactedShieldBlockInfo.PhysResistance / 100.f) +
		TakenDamageInfo.MagicDamage * (1.f - ImpactedShieldBlockInfo.MagicResistance / 100.f)
		) * TakenDamageInfo.DamagePercent;

	if (Damage == 0) return false; 

	int32 newHealth = health - Damage;

	RepcurrentHealth = newHealth;
	OnHealthChangedLeftOver.Broadcast(newHealth, maxHealth);

	newHealth = FMath::Clamp(newHealth, 0, maxHealth);
	health = newHealth;

	HealthChanged.Broadcast(health, maxHealth);

	if (health <= 0) {
		isDead = true;
		ActorDied.Broadcast(GetOwner());

		return true;
	}

	return false;
}

void UHealthComponent::Heal(float HealAmount)
{
	if (this && GetOwner()->GetLocalRole() != ROLE_Authority) {
		ServerHeal(HealAmount);
	}

	int32 newHealth = health + HealAmount;

	RepcurrentHealth = newHealth;
	OnHealthChangedLeftOver.Broadcast(newHealth, maxHealth);
	newHealth = FMath::Clamp(newHealth, 0, maxHealth);
	health = newHealth;
	HealthChanged.Broadcast(health, maxHealth);
}

void UHealthComponent::AddToAttribute(float ValueToAdd)
{
	FDamageInfo tempSave;
	tempSave.PhysDamage = ValueToAdd;
	TakeDamage(tempSave);
}

void UHealthComponent::OnRepHealth()
{
	health = RepcurrentHealth;
	maxHealth = FMath::Max(1, RepMaxHealth);

	OnHealthChangedLeftOver.Broadcast(health, maxHealth);

	health = FMath::Clamp(RepcurrentHealth, 0, maxHealth);

	HealthChanged.Broadcast(health, maxHealth);

	if (health <= 0) {
		isDead = true;
		ActorDied.Broadcast(GetOwner());
	}
}

void UHealthComponent::ServerHeal_Implementation(float HealAmount)
{
	Heal(HealAmount);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, RepcurrentHealth);
	DOREPLIFETIME(UHealthComponent, RepMaxHealth);
}

void UHealthComponent::ResetHealthComponent() {
	health = maxHealth;
	OnHealthChangedLeftOver.Broadcast(health, maxHealth);
	HealthChanged.Broadcast(health, maxHealth);
}

void UHealthComponent::SetUpHealthComponent()
{

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	// owner is EnemyCore
	if (Cast<AEnemyCore>(GetOwner())) {
		OwnerTagComponent = GetOwner()->GetComponentByClass<UGameplayTagManagerComp>();

		if (!OwnerTagComponent) UE_LOG(LogTemp, Error, TEXT("No gameplay tag in EnemyCore %s"), *GetOwner()->GetName());
		ResetHealthComponent();

		return;
	}

	// owner is player

	if (APlayerState* OwnerPlayerState = OwnerPawn->GetPlayerState()) {
		OwnerTagComponent = OwnerPlayerState->GetComponentByClass<UGameplayTagManagerComp>();

		if (!OwnerTagComponent) UE_LOG(LogTemp, Error, TEXT("No gameplay tag in player %s"), *GetOwner()->GetName());
	}
	ResetHealthComponent();

}

void UHealthComponent::SetIsDead(bool newVal)
{
	isDead = newVal;
}