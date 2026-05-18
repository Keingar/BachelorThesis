// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "multiplayerTEST/GameModeRelated/multiplayerTESTGameMode.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTeestHUD.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/Characters/multiplayerTESTCharacter.h"
#include "Net/UnrealNetwork.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTestGameStateBase.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTestPlayerState.h"
#include <Kismet/GameplayStatics.h>
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpectatorPawn.h"
#include "multiplayerTEST/Characters/Enemies/EnemyCore.h"
#include "multiplayerTEST/GameModeRelated/MultiCheatManager.h"

AMyPlayerController::AMyPlayerController() {
	isOpenInventory = false;
	CurrentSpectatedPawn = 0;
	CustomHUD = nullptr;


	CheatClass = UMultiCheatManager::StaticClass();
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CustomHUD = Cast<AMultiplayerTeestHUD>(GetHUD());
}

void AMyPlayerController::SpectateNextPawn()
{
	if (!GetPawn() || !Cast<ASpectatorPawn>(GetPawn())) {
		return;
	}

	if (CurrentSpectatedPawn >= GetWorld()->GetGameState<AMultiplayerTestGameStateBase>()->GetActivePlayerCount() - 1) {
		CurrentSpectatedPawn = 0;
	}
	else {
		CurrentSpectatedPawn = CurrentSpectatedPawn + 1;
	}

	ChooseSpectatedPawn();
}

void AMyPlayerController::SpectatePreviousPawn()
{
	if (!GetPawn() || !Cast<ASpectatorPawn>(GetPawn())) {
		return;
	}

	if (CurrentSpectatedPawn <= 0) {
		CurrentSpectatedPawn = GetWorld()->GetGameState<AMultiplayerTestGameStateBase>()->GetActivePlayerCount() - 1;
	}
	else {
		CurrentSpectatedPawn = CurrentSpectatedPawn - 1;
	}

	ChooseSpectatedPawn();
}

void AMyPlayerController::ChooseSpectatedPawn()
{
	int TemporaryCurrentPlayerIndex = 0;

	// when no one alive (can be changed to anything)++
	if (GetWorld()->GetGameState<AMultiplayerTestGameStateBase>()->GetActivePlayerCount() == 0) {
		return;
	}

	for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray) {
		APawn* CurrentPawn = PS->GetPawn();

		if (!CurrentPawn || Cast<AMultiplayerTestPlayerState>(PS)->IsDead == true || this == PS->GetPlayerController() || Cast<ASpectatorPawn>(CurrentPawn) || CurrentPawn == LastDeathCameraTarget) {
			continue;
		}

		if (TemporaryCurrentPlayerIndex == CurrentSpectatedPawn) {

			APawn* OwnerPawn = GetPawn();
			if (!OwnerPawn) {	
				return;
			}

			USceneComponent* AttachComponent = CurrentPawn->GetRootComponent();
			if (!AttachComponent) return;

			if (LastDeathCameraTarget) {
				if (UHealthComponent* HealthComponent = LastDeathCameraTarget->GetComponentByClass<UHealthComponent>())
				{
					HealthComponent->ActorDied.RemoveDynamic(this, &AMyPlayerController::OnDeathCameraTargetDied);
				}
			}

			if (UHealthComponent* HealthComponent = CurrentPawn->GetComponentByClass<UHealthComponent>())
			{
				HealthComponent->ActorDied.AddDynamic(this, &AMyPlayerController::OnDeathCameraTargetDied);
			}

			LastDeathCameraTarget = CurrentPawn;
			OwnerPawn->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepRelativeTransform);

			OwnerPawn->SetActorRelativeLocation(FVector::ZeroVector);
			
			OnSpectatedSwitched.Broadcast(PS);

			break;
		}


		TemporaryCurrentPlayerIndex++;
	}
}

AMultiplayerTeestHUD* AMyPlayerController::GetCustomHUD() const
{
	return CustomHUD;
}

bool AMyPlayerController::OnFightStart(AActor* newEnemy)
{
	if(!newEnemy) {
		UE_LOG(LogTemp, Warning, TEXT("Invalid newEnemy in OnFightStart in %s"), *this->GetName());
		return false;
	}
	
	if (ListOfEnemies.Contains(newEnemy)) {
		return false;
	}

	UHealthComponent* EnemyHealhComponent = newEnemy->GetComponentByClass<UHealthComponent>();
		
	if (!EnemyHealhComponent) {
		UE_LOG(LogTemp, Error, TEXT("Enemy has no health component can't start fight"));

		return false;
	}

	EnemyHealhComponent->ActorDied.AddDynamic(this, &AMyPlayerController::OnTargetDied); // need to change initial delegate
	newEnemy->OnDestroyed.AddDynamic(this, &AMyPlayerController::OnTargetIsDestroyed);

	if (ListOfEnemies.Num() == 0) {
		OnFightStarted.Broadcast();
	}


	AEnemyCore* huhEnemy = Cast<AEnemyCore>(newEnemy);

	if (huhEnemy && huhEnemy->GetIsBoss()) {
		OnBossFightStarted.Broadcast(newEnemy);
	}

	ListOfEnemies.Add(newEnemy);

	return true;
}

void AMyPlayerController::OnTargetDied(AActor* DeadEnemy)
{
	DeadEnemy->GetComponentByClass<UHealthComponent>()->ActorDied.RemoveDynamic(this, &AMyPlayerController::OnTargetDied);
	DeadEnemy->OnDestroyed.RemoveDynamic(this, &AMyPlayerController::OnTargetIsDestroyed);

	ListOfEnemies.Remove(DeadEnemy);

	if (ListOfEnemies.IsEmpty()) {
		APlayerState* MyPS = GetPlayerState<APlayerState>();
		if (MyPS) {
			MyPS->GetComponentByClass<UGameplayTagManagerComp>()->RemoveTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.InCombat")));
		}

		OnFigthStopped.Broadcast();

		AEnemyCore* huhEnemy = Cast<AEnemyCore>(DeadEnemy);
		if (huhEnemy && huhEnemy->GetIsBoss()) {
			OnBossFightStopped.Broadcast(DeadEnemy);
		}

		return;
	}
}

void AMyPlayerController::OnTargetIsDestroyed(AActor* DestroyedActor)
{
	ListOfEnemies.Remove(DestroyedActor);

	if (ListOfEnemies.IsEmpty()) {
		APlayerState* MyPS = GetPlayerState<APlayerState>();
		if (MyPS) {
			MyPS->GetComponentByClass<UGameplayTagManagerComp>()->RemoveTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.InCombat")));
		}

		OnFigthStopped.Broadcast();

		return;
	}
}

void AMyPlayerController::OnDeathCameraTargetDied(AActor* DeadTarget)
{
	ChooseSpectatedPawn();
}

void AMyPlayerController::OnOwnerDied()
{
	isOpenInventory = false;
	CurrentSpectatedPawn = 0;
	LastDeathCameraTarget = nullptr;

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_SpectatePawn);

	if (!ListOfEnemies.IsEmpty()) {
		for (AActor* Enemy : ListOfEnemies) {
			if (!Enemy) continue;
			Enemy->GetComponentByClass<UHealthComponent>()->ActorDied.RemoveDynamic(this, &AMyPlayerController::OnTargetDied);

			Enemy->OnDestroyed.RemoveDynamic(this, &AMyPlayerController::OnTargetIsDestroyed);

		}

		APlayerState* MyPS = GetPlayerState<APlayerState>();
		if (MyPS) {
			MyPS->GetComponentByClass<UGameplayTagManagerComp>()->RemoveTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.InCombat")));
		}

		ListOfEnemies.Empty();

		OnFigthStopped.Broadcast();
	}
	if (GetLocalRole() == ROLE_Authority) { SwitchToSpectatePawn(); };
	
	if (IsLocalController()) {
		DestroyHUD();
		TrySpectateImmidiatelyAfterDeath();
	}
}

void AMyPlayerController::SetIsOpenInventory_Implementation(bool newVal)
{
	isOpenInventory = newVal;
}

void AMyPlayerController::CreateHUD()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_SpectatePawn);

	if (!IsLocalController()) {
		return;
	}

	AMultiplayerTeestHUD* CharacterHUD = Cast<AMultiplayerTeestHUD>(GetHUD());

	if (CharacterHUD) {
		EnableInput(this);
		CharacterHUD->DrawWidgets();
	}
}


void AMyPlayerController::DestroyHUD()
{
	if (CustomHUD) {
		CustomHUD->RemoveAllWidgetsEvent();
	}
}

void AMyPlayerController::ResetHUDAfterBonfireRest()
{
	if (CustomHUD) {
		CustomHUD->ResetUIAfterBonfire();
	}
}

void AMyPlayerController::TrySpectateImmidiatelyAfterDeath()
{
	if (!GetPawn()) {
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_SpectatePawn, this, &AMyPlayerController::TrySpectateImmidiatelyAfterDeath, 0.1f, false);
		return;
	}

	if (!Cast<ASpectatorPawn>(GetPawn())) {
		return;
	}

	ChooseSpectatedPawn();
}

void AMyPlayerController::SwitchToSpectatePawn()
{
	if (GetLocalRole() != ROLE_Authority || !GetPawn()) { return; }

	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AGameModeBase>();
	if (!GameMode) return;

	TSubclassOf<ASpectatorPawn> SpectatorClass = GameMode->SpectatorClass;
	if (!SpectatorClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetPawn();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector SpawnLocationn = GetPawn()->GetActorLocation();
	FRotator SpawnRotationn = GetPawn()->GetActorRotation();

	APawn* Spectator = GetWorld()->SpawnActor<APawn>(
		SpectatorClass,
		SpawnLocationn,
		SpawnRotationn,
		SpawnParams
	);

	if (UPawnMovementComponent* MoveComp = Spectator->GetMovementComponent())
	{
		MoveComp->Deactivate();
	}


	if (Spectator)
	{
		APawn* OldPawwn = GetPawn();

		UnPossess();
		Possess(Spectator);
	}
}

void AMyPlayerController::ResetController()
{
	ListOfEnemies.Empty();
}