#include "multiplayerTESTGameMode.h"
#include "multiplayerTEST/Characters/multiplayerTESTCharacter.h"
#include "multiplayerTEST/Controllers/MyPlayerController.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTeestHUD.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTestPlayerState.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTestGameStateBase.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "multiplayerTEST/Interactable/PickUpActorCore.h"
#include "TimerManager.h"	
#include "Kismet/GameplayStatics.h"
#include "multiplayerTEST/RespawnStuff/DebugSpawnPoint.h"
#include "multiplayerTEST/GameplayInterfaces/EnvironmentInteractInterface.h"
#include "multiplayerTEST/Interactable/BonepileCore.h"
#include "multiplayerTEST/Subsystems/WorldSubsystems/RegistrySubsystem.h"
#include "multiplayerTEST/Subsystems/GameInstanceSubsystems/SaveGameSubsystem.h"
#include "multiplayerTEST/RespawnStuff/PlayerDefaultLocationSpawn.h"
#include "Components/CapsuleComponent.h"

AmultiplayerTESTGameMode::AmultiplayerTESTGameMode()
{
	RespawnTimer = 5.0f;
	ActivePlayer = 0;
	TimeBeforeResettingAfterFinishRound = 5.0;
}

void AmultiplayerTESTGameMode::BeginPlay()
{
	Super::BeginPlay();

	CustomGameState = Cast<AMultiplayerTestGameStateBase>(GameState);
	CacheSubsystems();
}

void AmultiplayerTESTGameMode::CacheSubsystems()
{
	UGameInstance* GI = GetGameInstance();
	if(!SaveGameSubsystem) SaveGameSubsystem = GI ? GI->GetSubsystem<USaveGameSubsystem>() : nullptr;
	if (!RegistrySubsystem) RegistrySubsystem = GetWorld() ? GetWorld()->GetSubsystem<URegistrySubsystem>() : nullptr;
}

void AmultiplayerTESTGameMode::DetermineSpawnLocation(AController* Controller, bool bIsFirstSpawn, FVector& OutLocation, FRotator& OutRotation)
{
#if WITH_EDITOR
	if (AActor* DebugSpawn = UGameplayStatics::GetActorOfClass(GetWorld(), ADebugSpawnPoint::StaticClass()))
	{
		OutLocation = DebugSpawn->GetActorLocation();
		OutRotation = DebugSpawn->GetActorRotation();
		if (!SaveGameSubsystem || !RegistrySubsystem)
		{
			CacheSubsystems();
		}

		return;
	}
#endif

	if(!SaveGameSubsystem || !RegistrySubsystem)
	{
		CacheSubsystems();
	}

	if (SaveGameSubsystem->isSavedTransformFromCurrentLevel()) {
		if (bIsFirstSpawn && GetFirstSpawnLocation(OutLocation, OutRotation))
		{
			return;
		}

		if (GetLocationOfSavedBonpile(OutLocation, OutRotation)) return;
	}

	GetDefaultSpawnLocation(OutLocation, OutRotation);
}

bool AmultiplayerTESTGameMode::GetFirstSpawnLocation(FVector& OutLocation, FRotator& OutRotator) {
	if (!SaveGameSubsystem) return false;

	return SaveGameSubsystem->GetSavedTransform(OutLocation, OutRotator);
}

bool AmultiplayerTESTGameMode::GetLocationOfSavedBonpile(FVector& OutLocation, FRotator& OutRotation)
{
	if (!SaveGameSubsystem || !RegistrySubsystem) return false;
	
	FName LastBonpileID;

	if (!SaveGameSubsystem->GetLastRestedBonpileID(LastBonpileID)) return false; 
	
	ABonepileCore* BonepileActor = Cast<ABonepileCore>(RegistrySubsystem->GetObjectByID(LastBonpileID));

	if (!BonepileActor) return false;
	
	OutLocation = BonepileActor->GetActorLocation() + FVector(0, 0, 40);
	OutRotation = BonepileActor->GetActorRotation();

	return true;
}

void AmultiplayerTESTGameMode::Respawn(AController* Controller)
{
	if (GetLocalRole() != ROLE_Authority || !Controller) {
		return;
	}

	ActivePlayer--;

	if (ActivePlayer == 0) {
		if (URegistrySubsystem* Registry = GetWorld()->GetSubsystem<URegistrySubsystem>())
		{
			Registry->ResetObjectsAndSpawners();
		}

		RespawnAllPlayers();
		CheckIfLostDuringBossFight();
	}
}

void AmultiplayerTESTGameMode::RespawnAllPlayers()
{
	ActivePlayer = 0; // reset and recount during respawn
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AController* Controller = It->Get();
		if (Controller)
		{
			Spawn(Controller);
		}
	}
}

void AmultiplayerTESTGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (Cast<AMultiplayerTestGameStateBase>(GameState)->GetCurrentGamePhase() != OpenWorld) {

		NewPlayer->GetPawn()->Destroy();

		if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(NewPlayer)) {
			PlayerController->CurrentSpectatedPawn = 0;

			ActivePlayer--;
			PlayerController->ChooseSpectatedPawn();
		}
	}
}
void AmultiplayerTESTGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (Exiting->IsLocalPlayerController()) {
		return;
	}

	ActivePlayer--;

	if (ActivePlayer == 0 ) {
		RespawnAllPlayers();
	}
}


AActor* AmultiplayerTESTGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
#if WITH_EDITOR
	if (AActor* DebugSpawn = UGameplayStatics::GetActorOfClass(GetWorld(), ADebugSpawnPoint::StaticClass()))
	{
		return DebugSpawn;
	}
#endif

	APlayerDefaultLocationSpawn* SpawnActor =
	Cast<APlayerDefaultLocationSpawn>(
		UGameplayStatics::GetActorOfClass(
			GetWorld(),
			APlayerDefaultLocationSpawn::StaticClass()
		)
	);

	if (SpawnActor) return SpawnActor;

	return Super::ChoosePlayerStart_Implementation(Player);
}

void AmultiplayerTESTGameMode::Spawn(AController* Controller) {
	FVector Location;
	FRotator Rotation;

	DetermineSpawnLocation(Controller, /*bIsFirstSpawn*/false, Location, Rotation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	if (AmultiplayerTESTCharacter* Pawn = Cast<AmultiplayerTESTCharacter>(GetWorld()->SpawnActor<APawn>(DefaultPawnClass, Location, FRotator::ZeroRotator, SpawnParams))) {
		if (APawn* CheckPawn = Controller->GetPawn()) {
			Controller->UnPossess();
			CheckPawn->Destroy();
		}

		Controller->Possess(Pawn);
		ActivePlayer++; // count newly spawned/alive player
	}
}

void AmultiplayerTESTGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	APawn* Pawn = NewPlayer->GetPawn();
	if (!Pawn)
	{
		return;
	}

	AMultiplayerTestPlayerState* PS = NewPlayer->GetPlayerState<AMultiplayerTestPlayerState>();
	const bool bFirstSpawn = PS ? !PS->bHasSpawned : false;

	FVector Loc;
	FRotator Rot;
	DetermineSpawnLocation(NewPlayer, bFirstSpawn, Loc, Rot);
	Pawn->TeleportTo(Loc + FVector(0.f, 0.f, 500.f), Rot, false, true);

	if (PS)
	{
		PS->bHasSpawned = true;
	}

	// Count initial/engine-driven spawns
	ActivePlayer++;
}

void AmultiplayerTESTGameMode::CheckIfLostDuringBossFight()
{
	if (CustomGameState->GetCurrentGamePhase() == BossFight) {

	//	CurrentBoss->BossWonFight(); // TODO: what is this? probably reset needed
		CustomGameState->OnBossFightEnded(ListOfCurrentBosses[0], false);

	}
}

void AmultiplayerTESTGameMode::AddActiveBoss(AEnemyCore* NewBoss)
{
	
	ListOfCurrentBosses.AddUnique(NewBoss);
}

// returns true if there is no active bossess at the moment
bool AmultiplayerTESTGameMode::RemoveActiveBoss(AEnemyCore* BossToRemove)
{
	ListOfCurrentBosses.RemoveSingle(BossToRemove);

	return ListOfCurrentBosses.IsEmpty(); // return true if empty
}

void AmultiplayerTESTGameMode::ResetPlayerCharacters()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AController* Controller = It->Get();
		if (!Controller) {
			UE_LOG(LogTemp, Error, TEXT("Invalid controller when tried to respawn "));
			return;
		}

		if (AmultiplayerTESTCharacter* PC = Cast<AmultiplayerTESTCharacter>(Controller->GetPawn())) {
			PC->ResetCharacter();
			return;
		}

		Spawn(Controller);
	}
}

void AmultiplayerTESTGameMode::TeleportAllPlayersToBonpile(FName BonpileID)
{
	if (GetLocalRole() != ROLE_Authority || BonpileID.IsNone())
	{
		return;
	}

	URegistrySubsystem* Registry = GetWorld()->GetSubsystem<URegistrySubsystem>();
	if (!Registry)
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportAllPlayersToBonpile: RegistrySubsystem missing"));
		return;
	}

	AActor* BonpileActor = Registry->GetObjectByID(BonpileID);
	ABonepileCore* TargetBonpile = Cast<ABonepileCore>(BonpileActor);
	if (!TargetBonpile)
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportAllPlayersToBonpile: Bonpile %s not found"), *BonpileID.ToString());
		return;
	}

	const FVector BaseLocation = TargetBonpile->GetTeleportLocation();
	const FRotator DestRotation = TargetBonpile->GetTeleportRotation();

	const float SpawnRadius = 120.f;
	const int32 NumPlayers = GetNumPlayers();
	int32 PlayerIndex = 0;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AController* Controller = It->Get();
		if (!Controller) continue;

		APawn* Pawn = Controller->GetPawn();
		if (!Pawn) continue;

		//Dynamic capsule height
		float CapsuleHalfHeight = 88.f;
		if (ACharacter* Char = Cast<ACharacter>(Pawn))
		{
			CapsuleHalfHeight = Char->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		}

		// Even circular distribution
		const float AngleStep = 360.f / FMath::Max(1, NumPlayers);
		const float AngleDeg = PlayerIndex * AngleStep;
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);

		FVector Offset = FVector(
			FMath::Cos(AngleRad) * SpawnRadius,
			FMath::Sin(AngleRad) * SpawnRadius,
			0.f
		);

		FVector AdjustedBaseLocation = BaseLocation + Offset;

		FVector TraceStart = AdjustedBaseLocation + FVector(0, 0, 300.f);
		FVector TraceEnd = AdjustedBaseLocation - FVector(0, 0, 800.f);

		FHitResult Hit;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Pawn);
		Params.bTraceComplex = false;

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			Params
		);

		//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 5.f, 0, 2.f);

		FVector FinalLocation = AdjustedBaseLocation;

		if (bHit)
		{
			FinalLocation = Hit.Location + FVector(0, 0, CapsuleHalfHeight +5);

		//	DrawDebugSphere(GetWorld(), Hit.Location, 20.f, 12, FColor::Red, false, 5.f);

		//	DrawDebugSphere(GetWorld(), FinalLocation, 25.f, 12, FColor::Blue, false, 5.f);
		}

		// Teleport
		if (IEnvironmentInteractInterface* Teleportable = Cast<IEnvironmentInteractInterface>(Pawn))
		{
			Teleportable->TeleportActor(FinalLocation, DestRotation, true);
		}

		PlayerIndex++;
	}

	//  Save system
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USaveGameSubsystem* SaveSubsystem = GI->GetSubsystem<USaveGameSubsystem>())
		{
			SaveSubsystem->SetLastRestedBonpile(BonpileID);
			SaveSubsystem->ForceSave();
		}
	}
}

void AmultiplayerTESTGameMode::GetDefaultSpawnLocation(
	FVector& OutLocation,
	FRotator& OutRotation)
{
	APlayerDefaultLocationSpawn* SpawnActor =
		Cast<APlayerDefaultLocationSpawn>(
			UGameplayStatics::GetActorOfClass(
				GetWorld(),
				APlayerDefaultLocationSpawn::StaticClass()
			)
		);

	if (SpawnActor)
	{
		OutLocation = SpawnActor->GetActorLocation();
		OutRotation = SpawnActor->GetActorRotation();
		return;
	}

	UE_LOG(LogTemp, Error,
		TEXT("GetDefaultSpawnLocation: No APlayerDefaultLocationSpawn found"));

	OutLocation = FVector::ZeroVector;
	OutRotation = FRotator::ZeroRotator;
}