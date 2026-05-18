// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWorldSpaceHealthBar.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/ProgressBar.h"
#include "multiplayerTEST/Components/HealthComponent.h"

void UPlayerWorldSpaceHealthBar::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Visible);
	NicknameText->SetVisibility(ESlateVisibility::Hidden);
	HealthBar->SetVisibility(ESlateVisibility::Hidden);
	PreviousHealthBar->SetVisibility(ESlateVisibility::Hidden);
}

void UPlayerWorldSpaceHealthBar::SetUpHealthDelegate(UHealthComponent* OwningHealthComponent)
{
	Super::SetUpHealthDelegate(OwningHealthComponent);
	SetUpOnSomeoneDeath(OwningHealthComponent);
}

void UPlayerWorldSpaceHealthBar::SetUpNickname(FText PlayerNickname, AActor* LocalPlayerr)
{
	if (NicknameText)
	{
		NicknameText->SetText(PlayerNickname);
	}
	else
	{
		NicknameText->SetText(FText::FromString("DefaultName"));
	}

	LocalPlayer = LocalPlayerr;
	WidgetOwner = GetOwningPlayerPawn();

	if(!LocalPlayer || !WidgetOwner.IsValid())
	{
		return;
	}

	UHealthComponent* HealthComponent = WidgetOwner->FindComponentByClass<UHealthComponent>();
	if (HealthComponent && LocalPlayer != WidgetOwner)
	{
		HealthComponent->ActorDied.AddDynamic(this, &UPlayerWorldSpaceHealthBar::OnSomeoneDeath);
	}

	GetWorld()->GetTimerManager().SetTimer(CheckOnstaclesTimer, this, &UPlayerWorldSpaceHealthBar::CheckObstacles, 0.3f, true);
}

void UPlayerWorldSpaceHealthBar::CheckObstacles()
{
	if (!LocalPlayer || !WidgetOwner.IsValid()) {
		CheckOnstaclesTimer.Invalidate(); 
		return;
	}


	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    FHitResult HitResult;

    FCollisionQueryParams TraceParams(FName(TEXT("LockOnTrace")), true, WidgetOwner.Get());
    TraceParams.AddIgnoredActor(WidgetOwner.Get());


    FVector Direction = (LocalPlayer->GetActorLocation() - WidgetOwner->GetActorLocation()).GetSafeNormal();
    FVector ExtendedEndPoint = LocalPlayer->GetActorLocation() + (Direction * 50.0f);

  //  DrawDebugLine(GetWorld(),WidgetOwner->GetActorLocation(), ExtendedEndPoint,FColor::Red,false, 5.0f,0,1.0f);
    bool bIsHitSuccessfull = GetWorld()->LineTraceSingleByChannel(HitResult, WidgetOwner->GetActorLocation(), ExtendedEndPoint, ECC_Visibility, TraceParams);
    if (bIsHitSuccessfull && LocalPlayer == HitResult.GetActor()) {
		NicknameText->SetVisibility(ESlateVisibility::Visible);

        return;
    }
	NicknameText->SetVisibility(ESlateVisibility::Hidden);
}

void UPlayerWorldSpaceHealthBar::SetDesiredVisibility()
{
	if (bisLockedOn || changedHealthRecently) {
		HealthBar->SetVisibility(ESlateVisibility::Visible);
		PreviousHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
	else {
		HealthBar->SetVisibility(ESlateVisibility::Hidden);
		PreviousHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPlayerWorldSpaceHealthBar::OnSomeoneDeath(AActor* DeadActor)
{
	CheckOnstaclesTimer.Invalidate();
	LocalPlayer = nullptr; // Clear the reference to the local player
	WidgetOwner = nullptr; // Clear the reference to the widget owner
	RemoveFromParent(); // Removes the widget from screen
}

void UPlayerWorldSpaceHealthBar::SetUpOnSomeoneDeath(UHealthComponent* OwningHealthComponent)
{
	if (OwningHealthComponent) {
		OwningHealthComponent->ActorDied.AddDynamic(this, &UPlayerWorldSpaceHealthBar::OnSomeoneDeath);
	}
}
