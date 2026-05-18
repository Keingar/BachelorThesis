// Fill out your copyright notice in the Description page of Project Settings.

#include "LockOnComponent.h"
#include "multiplayerTEST/Characters/multiplayerTESTCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "multiplayerTEST/GameplayInterfaces/LockOnInterface.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "multiplayerTEST/Components/PlayerComponents/MultiCharacterMovementComponent.h"
#include "multiplayerTEST/Components/PlayerComponents/MySpringArmComponent.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ULockOnComponent::ULockOnComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    LockOnTarget = nullptr;
    LockOnTargetMesh = nullptr;
    OwnerRef = nullptr;
    isLockedOn = false;
	bUsingCustomCameraParams = false;

    MaxLockOnDistance = 1500;
    XAxisSwitchSensivity = 1;
    YAxisSwitchSensivity = 1;
    DelayForSwitchAfterSwitch = 0.2f;
    LockOnCheckSphereRadius = 500;
    MinDistanceToSwitch2D = 60;
}

void ULockOnComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerRef = Cast<AmultiplayerTESTCharacter>(GetOwner());
    OwnerCamera = OwnerRef->GetCameraBoom();
    OwnerController = OwnerRef->GetController();


    OwnerRef->GetHealthComponent()->ActorDied.AddDynamic(this, &ULockOnComponent::StopLockOnAfterTargetDeath);
}

void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCamera) return;

    if (!isLockedOn) // return to default camera
    {
        OwnerCamera->TargetArmLength = FMath::FInterpTo(OwnerCamera->TargetArmLength, DefaultArmLength, DeltaTime, ZoomInterpSpeed);
        OwnerCamera->SocketOffset.Z = FMath::FInterpTo(OwnerCamera->SocketOffset.Z, DefaultSocketZ, DeltaTime, ZoomInterpSpeed);

        return;
    }

    if (!LockOnTargetMesh || !OwnerController || !OwnerRef)
        return;

    FVector MyLoc = OwnerRef->GetActorLocation();
    FVector TargetLoc = LockOnTargetMesh->GetSocketLocation(LockOnSocket.SocketName);
    FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(MyLoc, TargetLoc);

    if (!bUsingCustomCameraParams)
    {
        const FRotator CurrentRot = OwnerController->GetControlRotation();

        FRotator TargetRot = LookAtRot;
        TargetRot.Pitch = FMath::Max(
            FRotator::NormalizeAxis(TargetRot.Pitch - 10.f),
            MaxDownPitch
        );
        TargetRot.Roll = CurrentRot.Roll;

        OwnerController->SetControlRotation(
            FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 5.f)
        );
        return;
    }

    const float Distance = FVector::Dist(MyLoc, TargetLoc);
    const float HeightDiff = TargetLoc.Z - MyLoc.Z;

    // ---------- Pitch shaping ----------
    if (HeightDiff > 0.f)
    {
        const float AlphaPitch = FMath::Clamp(
            Distance / CurrentLockOnCameraParams.MinDistanceForFullPitch,
            0.f, 1.f
        );

        LookAtRot.Pitch = FMath::Lerp(10.f, LookAtRot.Pitch, AlphaPitch);
    }

    // ---------- Zoom shaping ----------
    const float AlphaZoom = FMath::Clamp(
        Distance / CurrentLockOnCameraParams.MinDistanceForZoomOut,
        0.f, 1.f
    );

    const float MaxExtraLength = DefaultArmLength *
        CurrentLockOnCameraParams.ExtraCameraLengthInPercent;

    OwnerCamera->TargetArmLength = FMath::FInterpTo(
        OwnerCamera->TargetArmLength,
        FMath::Lerp(DefaultArmLength + MaxExtraLength, DefaultArmLength, AlphaZoom),
        DeltaTime,
        ZoomInterpSpeed
    );

    OwnerCamera->SocketOffset.Z = FMath::FInterpTo(
        OwnerCamera->SocketOffset.Z,
        FMath::Lerp(CurrentLockOnCameraParams.MaxExtraZ, DefaultSocketZ, AlphaZoom),
        DeltaTime,
        ZoomInterpSpeed
    );

    // ---------- Rotation ----------
    const FRotator CurrentRot = OwnerController->GetControlRotation();

    FRotator TargetRot = LookAtRot;
    TargetRot.Pitch = FMath::Max(
        FRotator::NormalizeAxis(
            TargetRot.Pitch +
            FMath::Lerp(CurrentLockOnCameraParams.MaxDownPitch, 0.f, AlphaZoom) - 10.f
        ),
        MaxDownPitch
    );
    TargetRot.Roll = CurrentRot.Roll;

    OwnerController->SetControlRotation(
        FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 5.f)
    );

}


void ULockOnComponent::TryLockOn(int XDirection, int YDirection)
{
    if (isLockedOn && XDirection == 0 && YDirection == 0) { // if both are 0 0 then it was not called by trying to change target
        StopLockOn();
        return;
    }

    FVector FinalSpherePosition;

    if (!isLockedOn) { // use camera so possible to get target not only in front when when player actually looks
        const FRotator CameraRotation = OwnerRef->GetFollowCamera()->GetComponentRotation();

        FinalSpherePosition = OwnerRef->GetActorLocation() + CameraRotation.Vector() * LockOnCheckSphereRadius;
    }
    else {
        FinalSpherePosition = (UKismetMathLibrary::GetForwardVector(GetSphereRotator(GetZValue(XDirection, YDirection), YDirection)) * LockOnCheckSphereRadius) + GetSpherePosition(YDirection);
    }

    TArray<FHitResult> OutResults;

    //DrawDebugSphere(GetWorld(), FinalSpherePosition, LockOnCheckSphereRadius, 12, FColor::Red, false, 5.0f);
    if (!GetWorld()->SweepMultiByChannel(OutResults, FinalSpherePosition, FinalSpherePosition, FQuat::Identity, ECC_PhysicsBody, FCollisionShape::MakeSphere(LockOnCheckSphereRadius))) {
        return;
    }
    
    FSocketInfo LockOnSocketInfoTempSave;
    AActor* OutLockOnTempSave;

    GetLockOnInfo(OutResults, FinalSpherePosition, YDirection, LockOnSocketInfoTempSave, OutLockOnTempSave);

    if (!OutLockOnTempSave) {
        return;
    }

    UHealthComponent* LockOnTargetHealthComponent = OutLockOnTempSave->GetComponentByClass<UHealthComponent>();

    if (!LockOnTargetHealthComponent) {
        return;
    }

    ClearPreviousLockOn(); 
    SetLockOnTargetInfo(OutLockOnTempSave, LockOnSocketInfoTempSave);

    SetIsLockedOn(true);
    canChangeTargetAgain = false;

    LockOnTargetHealthComponent->ActorDied.AddDynamic(this, &ULockOnComponent::StopLockOnAfterTargetDeath);
    LockOnTarget->OnDestroyed.AddDynamic(this, &ULockOnComponent::StopLockOnAfterTargetDeath);

    ILockOnInterface* LockOnTargetInterface = Cast<ILockOnInterface>(LockOnTarget);

    if (LockOnTargetInterface->GetLockOnCameraParams().IsValidIndex(LockOnSocket.ChosenCameraCloseSettings)) {
        CurrentLockOnCameraParams = LockOnTargetInterface->GetLockOnCameraParams()[LockOnSocket.ChosenCameraCloseSettings];
        bUsingCustomCameraParams = true;
    }
    else {
		bUsingCustomCameraParams = false; // I could set it to false in clear function but then there is always will be small window of not having between different lock on
    }

    LockOnTargetInterface->OnLockedOnByActor_Implementation(LockOnSocket.SocketName);

    GetWorld()->GetTimerManager().SetTimer(CheckIfLockOnIsValid, this, &ULockOnComponent::CheckDistanceAndWalls, 0.2f, true);

    GetWorld()->GetTimerManager().SetTimer(LockOnSwitchTargetTimer, this, &ULockOnComponent::CalculationOfDirection, 0.3f, true);

    GetWorld()->GetTimerManager().SetTimer(TimerToChangeTargetAgain, this, &ULockOnComponent::CanChangeTargetAgain, DelayForSwitchAfterSwitch, false);

    OwnerRef->ChangeMovementOrientationForLockOn();
}

float ULockOnComponent::GetZValue(int XDirection, int YDirection) const
{
    if (XDirection < 0 || XDirection > 2 || YDirection < 0 || YDirection > 2)
    {
        UE_LOG(LogTemp, Error, TEXT("Incorrect int value (%d, %d) in GetZValue() Inside %s"),
            XDirection, YDirection, *GetName());
        return 0.f;
    }

    return LookupTable[XDirection][YDirection];
}

FRotator ULockOnComponent::GetSphereRotator(float SphereAngle, int YDirection) const
{
    if (!LockOnTarget) {
        FRotator SphereRotator = OwnerRef->GetActorRotation();

        SphereRotator.Pitch = OwnerRef->GetFollowCamera()->GetComponentRotation().Pitch;
        SphereRotator.Yaw = SphereRotator.Yaw - SphereAngle;

        return SphereRotator;
    }

    if (LockOnSocket.bLockOnDownOrForward && YDirection == 2 || LockOnSocket.bLockOnUpOrBehind && YDirection == 1) {
        return OwnerRef->GetActorRotation() + FRotator(SphereAngle + 90, 90, 0.f);
     }

    return OwnerRef->GetActorRotation() + FRotator(0.f, -SphereAngle, 0.f);
}

FVector ULockOnComponent::GetSpherePosition(int YDirection) const
{
    if (!LockOnTargetMesh) {
        return OwnerRef->GetActorLocation();
    }

    if (YDirection == 0 && LockOnSocket.bLockOnDownOrForward == true) {
        FVector SocketPositionWithActorHeight = LockOnTargetMesh->GetSocketLocation(LockOnSocket.SocketName);
        SocketPositionWithActorHeight.Z = OwnerRef->GetActorLocation().Z;

		return SocketPositionWithActorHeight;
    }

	return LockOnTargetMesh->GetSocketLocation(LockOnSocket.SocketName);
}

void ULockOnComponent::GetLockOnInfo(const TArray<FHitResult>& InResults, FVector InSphereCenter, int YDirection, FSocketInfo& OutChosenSocket, AActor*& OutLockedOnTarget) const
{
    float MinDistance = 999999;

    bool bShouldLockOn = false;

    int LockOnChosenIndex=0;

    int LockOnChosenIndexSocket=0;

    for (int32 HitIndex = 0; HitIndex < InResults.Num(); ++HitIndex)
    {
        const FHitResult& Hit = InResults[HitIndex];

        ILockOnInterface* HitActor = Cast<ILockOnInterface>(Hit.GetActor());
        
        if (!HitActor) {
            continue;
        }

        if (!HitActor->CanBeLockedOn_Implementation()) {
            continue;
        }

        USkeletalMeshComponent* HitActorMeshComponent = HitActor->GetLockOnTargetMesh_Implementation();

        const TArray<FSocketInfo> ListOfLockOnPoints = HitActor->GetLockOnSocketNames_Implementation();

        if (ListOfLockOnPoints.IsEmpty()) {
            continue;
        }
        for (int32 SocketIndex = 0; SocketIndex < ListOfLockOnPoints.Num(); ++SocketIndex) {
            const FName SocketName = ListOfLockOnPoints[SocketIndex].SocketName;

            if (!HitActorMeshComponent->DoesSocketExist(SocketName)) {
                continue;
            }

            if (LockOnTarget == Hit.GetActor() && LockOnSocket.SocketName == SocketName) {
                continue;
            }

            FHitResult WallCheckHit;

            HitActorMeshComponent->GetSocketLocation(SocketName);

            FVector CurrentSocketLocation = HitActorMeshComponent->GetSocketLocation(SocketName);

            FCollisionQueryParams TraceParams(FName(TEXT("LockOnTrace")), true, OwnerRef);
            TraceParams.AddIgnoredActor(OwnerRef);
            //DrawDebugLine(GetWorld(),OwnerRef->GetActorLocation(), CurrentSocketLocation,FColor::Red,false, 5.0f,0,1.0f);
            bool bIsHitSuccessfull = GetWorld()->LineTraceSingleByChannel(WallCheckHit, OwnerRef->GetActorLocation(), CurrentSocketLocation, ECC_PhysicsBody, TraceParams);

            if (!bIsHitSuccessfull || WallCheckHit.GetActor() == OwnerRef) {
                continue;
            }

            float newDistance = UKismetMathLibrary::Vector_Distance(CurrentSocketLocation, GetLockOnLocationVectorCheck());
            if (newDistance > MinDistance) {
                continue;
            }
            
            float DistanceFromSphereCenter = UKismetMathLibrary::Vector_Distance(InSphereCenter, CurrentSocketLocation);

            if (DistanceFromSphereCenter > LockOnCheckSphereRadius) {
                continue;
            }

            if (LockOnTarget && LockOnTargetMesh && LockOnTarget == Hit.GetActor() && UKismetMathLibrary::Vector_Distance2D(LockOnTargetMesh->GetSocketLocation(LockOnSocket.SocketName), CurrentSocketLocation) < MinDistanceToSwitch2D && YDirection == 0) {
                continue;
            }

            MinDistance = newDistance;

            LockOnChosenIndex = HitIndex;
            LockOnChosenIndexSocket = SocketIndex;

            bShouldLockOn = true;
        }
    }

    if (!bShouldLockOn) {
        OutChosenSocket.SocketName = FName("None");
		OutChosenSocket.bLockOnDownOrForward = false;
		OutChosenSocket.bLockOnUpOrBehind = false;

        OutLockedOnTarget = nullptr;
        return;
    }

    OutLockedOnTarget = InResults[LockOnChosenIndex].GetActor();

    ILockOnInterface* FinalLockOnTarget = Cast<ILockOnInterface>(OutLockedOnTarget);
    OutChosenSocket = FinalLockOnTarget->GetLockOnSocketNames_Implementation()[LockOnChosenIndexSocket];

}

FVector ULockOnComponent::GetLockOnLocationVectorCheck() const
{
    if (LockOnTarget) {
        return LockOnTarget->GetActorLocation();
    }

    return OwnerRef->GetActorLocation();
}

void ULockOnComponent::CheckDistanceAndWalls()
{
    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    if (UKismetMathLibrary::Vector_Distance(OwnerRef->GetActorLocation(), LockOnTarget->GetActorLocation()) > MaxLockOnDistance) {
        if (!TimerManager.IsTimerActive(StopLockOnByDistance)) {
            TimerManager.SetTimer(StopLockOnByDistance, this, &ULockOnComponent::StopLockOn, 2.0f, false);
        }
    }
    else {
        TimerManager.ClearTimer(StopLockOnByDistance);
        StopLockOnByDistance.Invalidate();
    }
    FHitResult HitResult;

    FCollisionQueryParams TraceParams(FName(TEXT("LockOnTrace")), true, OwnerRef);
    TraceParams.AddIgnoredActor(OwnerRef);



    FVector Direction = (LockOnTarget->GetActorLocation() - OwnerRef->GetActorLocation()).GetSafeNormal();
   // FVector ExtendedEndPoint = LockOnTarget->GetActorLocation() + (Direction * 50.0f);
    FVector ExtendedEndPoint = LockOnTargetMesh->GetSocketLocation(LockOnSocket.SocketName) + (Direction * 50.0f);
    // extended end point because hitboxe is mesh but not capsula and mesh is not always in the center due to animations (probably should use capsule as hitbox idk)

    //DrawDebugLine(GetWorld(),OwnerRef->GetActorLocation(), ExtendedEndPoint,FColor::Red,false, 5.0f,0,1.0f);
    bool bIsHitSuccessfull = GetWorld()->LineTraceSingleByChannel(HitResult, OwnerRef->GetActorLocation(), ExtendedEndPoint, ECC_PhysicsBody, TraceParams);


    if (bIsHitSuccessfull && LockOnTarget == HitResult.GetActor()) {
        TimerManager.ClearTimer(StopLockOnByWalls);
        StopLockOnByWalls.Invalidate();

        return;
    }

    if (!TimerManager.IsTimerActive(StopLockOnByWalls)) {
        TimerManager.SetTimer(StopLockOnByWalls, this, &ULockOnComponent::StopLockOn, 2.0f, false);
    }
}

void ULockOnComponent::CalculationOfDirection()
{
    int32 xDirection;
    int32 yDirection;

    float LastXValue = OwnerRef->GetLastXValue();
    float LastYValue = OwnerRef->GetLastYvalue();

    if (LastXValue < -12 * XAxisSwitchSensivity) {
        xDirection = 1;
    }
    else if (LastXValue > 12 * XAxisSwitchSensivity) {
        xDirection = 2;
    }
    else {
        xDirection = 0;
    }

    if (LastYValue < -6 * YAxisSwitchSensivity) {
        yDirection = 1;
    }
    else if (LastYValue > 6 * YAxisSwitchSensivity) {
        yDirection = 2;
    }
    else {
        yDirection = 0;
    }

    OwnerRef->SetLastXValue(0.f);
    OwnerRef->SetLastYValue(0.f);

    if (xDirection == 0 && yDirection == 0) {
        return;
    }
    
    TryLockOn(xDirection, yDirection);

}

void ULockOnComponent::CanChangeTargetAgain() // created only to assign to delegate
{
    canChangeTargetAgain = true;
}

void ULockOnComponent::StopLockOn()
{
	if (isLockedOn == false) return;

    SetIsLockedOn(false);

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    TimerManager.ClearTimer(StopLockOnByWalls);
    StopLockOnByWalls.Invalidate();

    TimerManager.ClearTimer(StopLockOnByDistance);
    StopLockOnByDistance.Invalidate();

    TimerManager.ClearTimer(CheckIfLockOnIsValid);
    CheckIfLockOnIsValid.Invalidate();

    TimerManager.ClearTimer(LockOnSwitchTargetTimer);
    LockOnSwitchTargetTimer.Invalidate();

    Cast<ILockOnInterface>(LockOnTarget)->StopLockOn_Implementation();

    LockOnTarget->OnDestroyed.RemoveDynamic(this, &ULockOnComponent::StopLockOnAfterTargetDeath);

    if (UHealthComponent* HealthComp = LockOnTarget->GetComponentByClass<UHealthComponent>()) {
        HealthComp->ActorDied.RemoveDynamic(this, &ULockOnComponent::StopLockOnAfterTargetDeath);
    }

    OwnerRef->SetDefaultMovementOrientation();

    SetLockOnTargetInfo(nullptr, FSocketInfo{});
    LockOnSocket = FSocketInfo();
    bUsingCustomCameraParams = false;
	CurrentLockOnCameraParams = FLockOnCameraParams();
}

void ULockOnComponent::ClearPreviousLockOn()
{
    if (!isLockedOn) return;

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    TimerManager.ClearTimer(StopLockOnByWalls);
    StopLockOnByWalls.Invalidate();

    TimerManager.ClearTimer(StopLockOnByDistance);
    StopLockOnByDistance.Invalidate();

    TimerManager.ClearTimer(CheckIfLockOnIsValid);
    CheckIfLockOnIsValid.Invalidate();

    TimerManager.ClearTimer(LockOnSwitchTargetTimer);
    LockOnSwitchTargetTimer.Invalidate();

    Cast<ILockOnInterface>(LockOnTarget)->StopLockOn_Implementation();

    if (UHealthComponent* HealthComp = LockOnTarget->GetComponentByClass<UHealthComponent>()) {
        HealthComp->ActorDied.RemoveDynamic(this, &ULockOnComponent::StopLockOnAfterTargetDeath);
    }

    LockOnTargetMesh = nullptr;
    LockOnTarget = nullptr;
    LockOnSocket = FSocketInfo();
}

void ULockOnComponent::StopLockOnAfterTargetDeath(AActor* DeadTarget)
{
    StopLockOn();
}

void ULockOnComponent::SetIsLockedOn(bool NewIsLockedOn)
{
    if (OwnerRef->GetLocalRole() != ROLE_Authority) {
        SetIsLockedOnServer(NewIsLockedOn);
    }

    isLockedOn = NewIsLockedOn;
}

void ULockOnComponent::SetIsLockedOnServer_Implementation(bool NewIsLockedOn)
{
    SetIsLockedOn(NewIsLockedOn);
}

void ULockOnComponent::SetLockOnTargetInfo(AActor* NewLockOnTarget, FSocketInfo newLockOnSocket)
{
    if (OwnerRef->GetLocalRole() != ROLE_Authority) {
        SetLockOnTargetInfoServer(NewLockOnTarget, newLockOnSocket);
    }

    LockOnTarget = NewLockOnTarget;
    LockOnSocket = newLockOnSocket;

    if (NewLockOnTarget) {
        ILockOnInterface* LockOnTargetInterface = Cast<ILockOnInterface>(NewLockOnTarget);

        LockOnTargetMesh = LockOnTargetInterface->GetLockOnTargetMesh_Implementation();
    }
    else {
		LockOnTargetMesh = nullptr;
    }

    if (UMultiCharacterMovementComponent* OwnerMoveComp = OwnerRef->GetComponentByClass<UMultiCharacterMovementComponent>()) {
		OwnerMoveComp->LockOnMesh = LockOnTargetMesh;
        OwnerMoveComp->LockOnSocket = newLockOnSocket.SocketName;
    }
}

void ULockOnComponent::SetLockOnTargetInfoServer_Implementation(AActor* NewLockOnTarget, FSocketInfo newLockOnSocket)
{
    SetLockOnTargetInfo(NewLockOnTarget, newLockOnSocket);
}

FVector ULockOnComponent::GetLockOnSocketTargetLocation()
{
    if (!LockOnTargetMesh) {
        return FVector(0,0,0);
	}
	return LockOnTargetMesh->GetSocketLocation(LockOnSocket.SocketName);
}