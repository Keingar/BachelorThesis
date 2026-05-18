#include "NameTagComponent.h"
#include "multiplayerTEST/GameModeRelated/MultiplayerTestPlayerState.h"

UNameTagComponent::UNameTagComponent()
{
}

void UNameTagComponent::CheckNameTagDesiredVisibility()
{
	if (GetWorld()->GetNumPlayerControllers() <= 1) {
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AController* CurrentController = It->Get();
		if (!CurrentController)
		{
			continue;
		}

		AActor* CurrentActor = CurrentController->GetPawn();

		if (!CurrentActor || CurrentActor == GetOwner()) {
			continue;
		}

		FHitResult HitResult;

		FCollisionQueryParams TraceParams(FName(TEXT("LockOnTrace")), true, GetOwner());
		TraceParams.AddIgnoredActor(GetOwner());

		FVector Direction = (CurrentController->GetPawn()->GetActorLocation() - CurrentController->GetPawn()->GetActorLocation()).GetSafeNormal();
		FVector ExtendedEndPoint = CurrentActor->GetActorLocation() + (Direction * 50.0f);

		// extended end point because hitboxe is mesh but not capsula and mesh is not always in the center due to animations (probably should use capsule as hitbox idk)

		//  DrawDebugLine(GetWorld(),OwnerRef->GetActorLocation(), ExtendedEndPoint,FColor::Red,false, 5.0f,0,1.0f);
		bool bIsHitSuccessfull = GetWorld()->LineTraceSingleByChannel(HitResult, GetOwner()->GetActorLocation(), ExtendedEndPoint, ECC_Visibility, TraceParams);

		UNameTagComponent* CurretntNameTagComponent = Cast<UNameTagComponent>(CurrentActor->GetComponentByClass<UNameTagComponent>());

		if (bIsHitSuccessfull && HitResult.GetActor() == CurrentActor) {

			CurretntNameTagComponent->SetVisibility(true);
			continue;
		}

		CurretntNameTagComponent->SetVisibility(false);
	}
}

void UNameTagComponent::SetUpNameTags()
{
	if (!Cast<APawn>(GetOwner())->IsLocallyControlled()) {
		return;
	}

	APawn* OwnerRef = Cast<APawn>(GetOwner());

	if (!OwnerRef) {
		return;
	}

	AMultiplayerTestPlayerState* PlayerState = Cast<AMultiplayerTestPlayerState>(OwnerRef->GetPlayerState());

	if (!PlayerState || PlayerState->GetCharacterName().IsEmpty()) {
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UNameTagComponent::SetUpNameTags, 0.1f, false);

		return;
	}

	NameTagText = PlayerState->GetCharacterName();

	GetWorld()->GetTimerManager().SetTimer(CheckNameTagDesiredVisibilityTimer, this, &UNameTagComponent::CheckNameTagDesiredVisibility, 0.2f, false);
}