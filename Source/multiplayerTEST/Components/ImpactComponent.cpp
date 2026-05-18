// Fill out your copyright notice in the Description page of Project Settings.


#include "ImpactComponent.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "multiplayerTEST/Components/StaminaComponent.h"
#include "multiplayerTEST/Controllers/CoreAIController.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/TagManagerOwner.h"
#include "multiplayerTEST/GameplayInterfaces/PlayerInformationInterface.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/GhostAnimInstanceSyncInterface.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/GetOwningMainMesh.h"
#include "multiplayerTEST/Items/Weapon.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"


UImpactComponent::UImpactComponent()
{
    maxPoise = 100.f;
    currentPoise = maxPoise;
    StaggerTag = FGameplayTag::RequestGameplayTag(FName("StatusEffect.RestrictiveEffects.Staggered"));
}

void UImpactComponent::BeginPlay()
{
    Super::BeginPlay();

    currentPoise = maxPoise;

    OwnerCharacter = GetOwner();

    if (OwnerCharacter && OwnerCharacter->GetClass()->ImplementsInterface(UGetOwningMainMesh::StaticClass()))
    {
        if (USkeletalMeshComponent* MainMesh =
            Cast<USkeletalMeshComponent>(
                IGetOwningMainMesh::Execute_GetOwningMainMesh(OwnerCharacter)))
        {
            OwnerAnimInstance = MainMesh->GetAnimInstance();
        }
    }

    if (OwnerCharacter) {
        OwnerStaminaComponent = OwnerCharacter->GetComponentByClass<UStaminaComponent>();
        OwnerHealthComponent = OwnerCharacter->GetComponentByClass<UHealthComponent>();

        OwnerCapsule = OwnerCharacter->GetComponentByClass<UCapsuleComponent>();

        if(APawn* OwnerPawn = Cast<APawn>(OwnerCharacter)) {
            OwnerAIController = Cast<ACoreAIController>(OwnerPawn->GetController());
		}
    }

    SetUpOwnerTagManagerRef();
}

void UImpactComponent::SetUpOwnerTagManagerRef()
{
    if (GetOwner()->GetClass()->ImplementsInterface(UTagManagerOwner::StaticClass()))
    {
        OwnerTagComp = ITagManagerOwner::Execute_GetTagManager(GetOwner());

        if (!OwnerTagComp)
        {
            FTimerHandle RetryTimerHandle;
            GetWorld()->GetTimerManager().SetTimer(RetryTimerHandle, this, &UImpactComponent::SetUpOwnerTagManagerRef, 0.1f, false);

            return;
        }
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Owner doesn't have tag manager while have hit component in %s"), *GetOwner()->GetName());
    }
}

bool UImpactComponent::isBlocking(AActor* ActorToCheck, float blockAngleToCheck)
{
    if (!OwnerTagComp || !OwnerTagComp->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.RestrictiveEffects.Blocking"))))
    {
        return false;
    }

    if (!OwnerCharacter) return false;

    if (!OwnerCapsule) return false;

    FVector CapsuleLocation = OwnerCapsule->GetComponentLocation();

    double Dot = FVector::DotProduct(OwnerCapsule->GetForwardVector(), (ActorToCheck->GetActorLocation() - CapsuleLocation).GetSafeNormal());

    Dot = FMath::Clamp(Dot, -1.0, 1.0);

    double AngleDeg = FMath::RadiansToDegrees(FMath::Acos(Dot));

    return AngleDeg < 60.0;
}

void UImpactComponent::getCurrentBlockInfo(float* blockAngleInDegrees, FBlockInfo& InBlockInfo)
{
    if (!blockAngleInDegrees || !GetOwner() || !OwnerTagComp || !GetOwner()->GetClass()->ImplementsInterface(UPlayerInformationInterface::StaticClass()))
    {
        return;
    }

    if (OwnerTagComp->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.UnrestrictiveEffects.TwoHandingWeapon")))) {
        UWeapon* RHWeapon = Cast<UWeapon>(IPlayerInformationInterface::Execute_GetWeaponRH1(GetOwner()));
        if (RHWeapon)
        {
            InBlockInfo = RHWeapon->GetItemDamageResistance();
            *blockAngleInDegrees = RHWeapon->GetBlockAngleInDegrees();
        }

        return;
    }

    UWeapon* LHWeapon = Cast<UWeapon>(IPlayerInformationInterface::Execute_GetWeaponLH1(GetOwner()));
    if (LHWeapon)
    {
        InBlockInfo = LHWeapon->GetItemDamageResistance();
        *blockAngleInDegrees = LHWeapon->GetBlockAngleInDegrees();
    }
}

void UImpactComponent::PlayMontageForOwnerAndItsGhost(UAnimMontage* MontageToPlay)
{
    if (OwnerGhostAnimInstance) {
        OwnerGhostAnimInstance->Montage_Play(MontageToPlay);
    }

    OwnerAnimInstance->Montage_Play(MontageToPlay);
}

void UImpactComponent::GotHit(const FHitResult& Hit, AActor* InstigatorActor, FDamageInfo HitDamage)
{
    if (!OwnerCharacter) { return; }
    
    float currentBlockAngleInDegrees = 0; 
    FBlockInfo ShieldBlockInfo;
    getCurrentBlockInfo(&currentBlockAngleInDegrees, ShieldBlockInfo);

    bool isBlockedAttack = isBlocking(InstigatorActor, currentBlockAngleInDegrees);

    FBlockInfo CorrectBlockInfo;

    if (isBlockedAttack) {
        CorrectBlockInfo = ShieldBlockInfo;
    }

    if (OwnerAIController) {
        OwnerAIController->CheckSensedActor(InstigatorActor);
    }

	if (!OwnerHealthComponent) return;

    if (OwnerHealthComponent->TakeDamage(HitDamage, CorrectBlockInfo)) {
        if(bPlayImpactAnimationOnDeath) SelectAndPlayImpactAnimation(Hit, InstigatorActor, isBlockedAttack);

        return;  // if died don't play impact animation
    }

    if (HitDamage.ImpactInfo.CustomImpactMontage && OwnerAnimInstance) {
        PlayMontageForOwnerAndItsGhost(HitDamage.ImpactInfo.CustomImpactMontage);
        return;
    }

    // if block then consume stamina and if consumed all stamina don't play impact only block break
    if (isBlockedAttack && OwnerStaminaComponent) { 
        if (OwnerStaminaComponent->AddStamina(-HitDamage.ImpactInfo.StaminaDamageOnBlock) == 0) {
            OnBlockBreak.Broadcast();
            return;
        }
    }

    if (!TakePoiseDamage(HitDamage.ImpactInfo.PoiseDamage)) {
        SelectAndPlayImpactAnimation(Hit, InstigatorActor, isBlockedAttack);
    }
}

void UImpactComponent::SelectAndPlayImpactAnimation(const FHitResult& Hit, AActor* InstigatorActor, bool bBlockedAttack)
{
    if (!OwnerCharacter || !InstigatorActor)
        return;

    FVector OwnerLocation = OwnerCharacter->GetActorLocation();

    //DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 50, FColor::Red, false, 5);

    FVector HitLocation = Hit.ImpactPoint;
    FVector ToHit = (HitLocation - OwnerLocation).GetSafeNormal();

    FVector Forward = OwnerCharacter->GetActorForwardVector();
    FVector Right = OwnerCharacter->GetActorRightVector();

    float ForwardDot = FVector::DotProduct(Forward, ToHit);
    float RightDot = FVector::DotProduct(Right, ToHit);
    float AngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));

    OnGotHit.Broadcast(InstigatorActor, AngleDegrees);

    if (bBlockedAttack) {
        OnGotBlockedHit.Broadcast();
        return;
    }

    float VerticalOffset = HitLocation.Z - OwnerLocation.Z;
    if (VerticalOffset > TopHitThreshold)
    {
        OnGotHitFromTop.Broadcast();
        return;
    }

    if (FMath::Abs(AngleDegrees) <= 30.0f)
    {
        OnGotHitFromStraight.Broadcast();
    }
    else if (AngleDegrees > 0) // any angle bigger then 0 is from right side
    {
        OnGotHitFromRight.Broadcast();
    }
    else
    {
        OnGotHitFromLeft.Broadcast();
    }
}

bool UImpactComponent::TakePoiseDamage(float PoiseDamage)
{
    if (!bCanBeStaggered || !OwnerTagComp) return false;

    if (OwnerTagComp->GetTags().HasTag(StaggerTag)) return true; // already staggered

    currentPoise -= PoiseDamage;
    if (currentPoise > 0) {
        return false;
    }

    OwnerTagComp->AddTag(StaggerTag);
    currentPoise = maxPoise; // its fine to do here instead of end stagger because its not supposed to change until end of stagger anyway
    OnStaggerStart.Broadcast();
    return true;
}

bool UImpactComponent::bEnemyCanStaggerHitCheck(AActor* InstigatorActor)
{
    if (!InstigatorActor)
        return false;

    // ---- 1. Check staggered status tag ----
    if (!OwnerTagComp ||
        !OwnerTagComp->GetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.RestrictiveEffects.Staggered.CanBeStaggeredAttack"))))
    {
        return false;
    }

    // ---- 2. Check distance (ignore height) ----
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector InstigatorLocation = InstigatorActor->GetActorLocation();

  //  FVector HitboxForStaggerLocation = OwnerLocation + StaggerCritHitBoxCenter;
    FVector HitboxForStaggerLocation =
        GetOwner()->GetActorTransform().TransformPosition(StaggerCritHitBoxCenter);
    float Distance = FVector::Dist2D(InstigatorLocation, HitboxForStaggerLocation);  

    if (Distance > StaggerCritHitBoxRadius)
    {
        // Outside crit hit radius
        return false;
    }

    // ---- 3. Check angle ----
    // Direction from owner to instigator
    FVector ToInstigator = (InstigatorLocation - HitboxForStaggerLocation).GetSafeNormal();

    // Forward direction of the owner (XY only)
    FVector Forward = GetOwner()->GetActorForwardVector();
    Forward.Z = 0.f;
    Forward.Normalize();

    // Angle between forward and direction to instigator
    float CosTheta = FVector::DotProduct(Forward, ToInstigator);
    float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(CosTheta));

    if (AngleDeg > AcceptableAngle * 0.5f) // AcceptableAngle is full cone
    {
        // Outside acceptable angle
        return false;
    }

    // ---- All checks passed ----

    //DrawDebugCircle(GetWorld(), StaggerCritHitBoxCenter, StaggerCritHitBoxRadius, 32, FColor::Green, false, 2.f, 0, 2.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
    //DrawDebugLine(GetWorld(), StaggerCritHitBoxCenter, StaggerCritHitBoxCenter + Forward * StaggerCritHitBoxRadius, FColor::Blue, false, 2.f, 0, 2.f);

    return true;
}

void UImpactComponent::ResetImpactComponent()
{
    currentPoise = maxPoise;
}
