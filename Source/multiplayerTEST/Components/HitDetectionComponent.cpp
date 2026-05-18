// Fill out your copyright notice in the Description page of Project Settings.


#include "HitDetectionComponent.h"
#include "multiplayerTEST/Items/Weapon.h"
#include "GameFramework/Character.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/GetOwningMainMesh.h"
#include "multiplayerTEST/Components/GameplayTagManagerComp.h"
#include "multiplayerTEST/GameplayInterfaces/InfoGetters/TagManagerOwner.h"
#include "multiplayerTEST/Components/ImpactComponent.h"
#include "multiplayerTEST/Components/HealthComponent.h"
#include "TimerManager.h"
#include "Engine/OverlapResult.h"

UHitDetectionComponent::UHitDetectionComponent()
{
	TagsThatBlockHit.AddTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.Buffs.Immune")));
	TagsThatBlockHit.AddTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.Buffs.Immune.RollImmune")));
	TagsThatBlockHit.AddTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.Buffs.Immune.OtherImmune")));
	TagsThatBlockHit.AddTag(FGameplayTag::RequestGameplayTag(FName("StatusEffect.RestrictiveEffects.Dead")));

	SetComponentTickEnabled(false);
}

void UHitDetectionComponent::BeginPlay() {
	Super::BeginPlay();

	SetUpOwnerRef();
}

void UHitDetectionComponent::SetUpOwnerRef() {
	AActor* OwnerRef = GetOwner();

	if (!OwnerRef) {
		UE_LOG(LogTemp, Error, TEXT("Invalid Owner in HitComponent"))
		return; 
	}
	SetUpOwnerTagManagerRef();

	if (ACharacter* OwnerSaved = Cast<ACharacter>(OwnerRef)) {
		OwnerMesh = OwnerSaved->GetMesh();
		return;
	}

	if (GetOwner()->GetClass()->ImplementsInterface(UGetOwningMainMesh::StaticClass()))
	{
		UMeshComponent* TempMesh = IGetOwningMainMesh::Execute_GetOwningMainMesh(OwnerRef);
		if (TempMesh)
		{
			OwnerMesh = TempMesh;
			return;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Couldn't find Mesh Component for hit component %s"), *OwnerRef->GetName());

}

void UHitDetectionComponent::SetUpOwnerTagManagerRef()
{
	if (GetOwner()->GetClass()->ImplementsInterface(UTagManagerOwner::StaticClass()))
	{
		OwnerTagComp = ITagManagerOwner::Execute_GetTagManager(GetOwner());

		if (!OwnerTagComp)
		{
			FTimerHandle RetryTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(RetryTimerHandle, this, &UHitDetectionComponent::SetUpOwnerTagManagerRef, 0.1f , false);

			return;
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Owner doesn't have tag manager while have hit component in %s"), *GetOwner()->GetName());
	}
}


void UHitDetectionComponent::GetNewWeaponData(UWeapon* NewWeapon, USceneComponent* SelectedWeapon)
{
	AllWeaponsFullHitBoxes.RemoveAll(
		[SelectedWeapon](const FFullWeaponHitBox& WeaponData)
		{
			return WeaponData.WeaponComponent == SelectedWeapon;
		}
	);

	FFullWeaponHitBox CurrentWeaponFullHitBox;

	CurrentWeaponFullHitBox.HitBoxesGroup.HitBoxes = *NewWeapon->GetHitBoxes();
	CurrentWeaponFullHitBox.HitBoxesGroup.SocketName = SelectedWeapon->GetAttachSocketName();

	CurrentWeaponFullHitBox.WeaponComponent = SelectedWeapon;

	AllWeaponsFullHitBoxes.Add(CurrentWeaponFullHitBox);
}

void UHitDetectionComponent::StartNewSocketCollision(int HitBoxChoice)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	if (!SocketHitBoxData.IsValidIndex(HitBoxChoice))
	{
		UE_LOG(LogTemp, Error, TEXT("HitDetectionComponent Start new socket collision fail. Out of bounds hit box choice"));
		return;
	}
	if (!OwnerMesh) {
		UE_LOG(LogTemp, Error, TEXT("invalid mesh in StartNewSocketCollision inside hit detection component"));
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(SocketHitBoxData[HitBoxChoice].HitBoxCollisionTimer)) {
		UE_LOG(LogTemp, Error, TEXT("Timer for socket %d is already active!"), HitBoxChoice);
		return;
	}

	FTimerDelegate TimerDel;

	SocketHitBoxData[HitBoxChoice].QueryParams.AddIgnoredActor(GetOwner());
	
	SocketHitBoxData[HitBoxChoice].bUsePreviousSocketDetection = false;

	TimerDel.BindUObject(this, &UHitDetectionComponent::PerformCollisionDetection, HitBoxChoice);

	// Start a looping timer for THIS weapon
	GetWorld()->GetTimerManager().SetTimer(
		SocketHitBoxData[HitBoxChoice].HitBoxCollisionTimer,
		TimerDel,
		0.016f,  
		true 
	);
}

void UHitDetectionComponent::StopSocketCollision(int HitBoxChoice)
{
	if (!SocketHitBoxData.IsValidIndex(HitBoxChoice)) return;

	GetWorld()->GetTimerManager().ClearTimer(SocketHitBoxData[HitBoxChoice].HitBoxCollisionTimer);
	
	SocketHitBoxData[HitBoxChoice].QueryParams.ClearIgnoredSourceObjects();
}

void UHitDetectionComponent::DoSocketCollisionOnce(int HitBoxChoice)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	if (!SocketHitBoxData.IsValidIndex(HitBoxChoice))
	{
		UE_LOG(LogTemp, Error, TEXT("HitDetectionComponent Start new socket collision fail. Out of bounds hit box choice"));
		return;
	}

	if (!OwnerMesh) {
		UE_LOG(LogTemp, Error, TEXT("invalid mesh in StartNewSocketCollision inside hit detection component"));
	}

	SocketHitBoxData[HitBoxChoice].QueryParams.AddIgnoredActor(GetOwner());

	SocketHitBoxData[HitBoxChoice].bUsePreviousSocketDetection = false;

	PerformCollisionDetection(HitBoxChoice);

	SocketHitBoxData[HitBoxChoice].QueryParams.ClearIgnoredSourceObjects();
}

void UHitDetectionComponent::StartWeaponCollision(USceneComponent* SelectedWeapon, FDamageInfo AttackDamageInfo) {
	if (GetOwnerRole() != ROLE_Authority) return;

	if (!OwnerMesh) {
		UE_LOG(LogTemp, Error, TEXT("invalid mesh in StartNewSocketCollision inside hit detection component"));
	}

	int32 Index = AllWeaponsFullHitBoxes.IndexOfByPredicate(
		[SelectedWeapon](const FFullWeaponHitBox& WeaponData)
		{
			return WeaponData.WeaponComponent == SelectedWeapon;
		}
	);

	if (Index == INDEX_NONE)
	{
		UE_LOG(LogTemp, Error, TEXT("Weapon data not found! Was it set up using GetNewWeaponData before ?"));
		return;
	}

	FFullWeaponHitBox& FoundWeapon = AllWeaponsFullHitBoxes[Index];

	if (GetWorld()->GetTimerManager().IsTimerActive(FoundWeapon.HitBoxesGroup.HitBoxCollisionTimer)) {
		UE_LOG(LogTemp, Error, TEXT("Timer for weapon socket %d is already active!"), Index);
		return;
	}

	FTimerDelegate TimerDel;

	FoundWeapon.HitBoxesGroup.QueryParams.AddIgnoredActor(GetOwner());
	AllWeaponsFullHitBoxes[Index].HitBoxesGroup.currentHitBoxDamageInfo = AttackDamageInfo;
	// Bind function + weapon parameter

	AllWeaponsFullHitBoxes[Index].HitBoxesGroup.bUsePreviousSocketDetection = false;

	TimerDel.BindUObject(this, &UHitDetectionComponent::PerformCollisionDetectionForWeapon, Index);
	// Start a looping timer for THIS weapon
	GetWorld()->GetTimerManager().SetTimer(
		FoundWeapon.HitBoxesGroup.HitBoxCollisionTimer,
		TimerDel,
		0.016f,   
		true    
	);
}

void UHitDetectionComponent::StopWeaponCollision(USceneComponent* SelectedWeapon) 
{
	FFullWeaponHitBox* FoundFullWeaponHitBox = AllWeaponsFullHitBoxes.FindByPredicate(
		[SelectedWeapon](const FFullWeaponHitBox& WeaponData)
		{
			return WeaponData.WeaponComponent == SelectedWeapon;
		}
	);

	if (!FoundFullWeaponHitBox) {
		UE_LOG(LogTemp, Error, TEXT("Timer for weapon socket %s is already active!"), *SelectedWeapon->GetName());

		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(FoundFullWeaponHitBox->HitBoxesGroup.HitBoxCollisionTimer);

	FoundFullWeaponHitBox->HitBoxesGroup.QueryParams.ClearIgnoredSourceObjects();
}

void UHitDetectionComponent::PerformCollisionDetection(int HitBoxChoice)
{
	if (SocketHitBoxData.IsEmpty()) return;
	if (!OwnerMesh) {
		UE_LOG(LogTemp, Error, TEXT("Invalid owner mesh couldn't perform collision detection"));
		return;
	}

	for (const FHitBoxData& HitBoxData : SocketHitBoxData[HitBoxChoice].HitBoxes) {
		FVector CurrentSocketLocation;
		FQuat WorldRotation;
			
		if (OwnerMesh->DoesSocketExist(SocketHitBoxData[HitBoxChoice].SocketName)) {
			CurrentSocketLocation = OwnerMesh->GetSocketLocation(SocketHitBoxData[HitBoxChoice].SocketName);
			WorldRotation = OwnerMesh->GetSocketRotation(SocketHitBoxData[HitBoxChoice].SocketName).Quaternion();
		}
		else {
			CurrentSocketLocation = OwnerMesh->GetComponentLocation();
			WorldRotation = OwnerMesh->GetComponentQuat();
		}

		CurrentSocketLocation += WorldRotation.RotateVector(HitBoxData.RelativeLocation);
		WorldRotation = WorldRotation * HitBoxData.RelativeRotation.Quaternion();

		FVector StartingCollisionDetectionLocation; // for sweep start location
		if (SocketHitBoxData[HitBoxChoice].bUsePreviousSocketDetection) {
			StartingCollisionDetectionLocation = SocketHitBoxData[HitBoxChoice].PreviousSocketLocation;
		}
		else {
			StartingCollisionDetectionLocation = CurrentSocketLocation;
			SocketHitBoxData[HitBoxChoice].bUsePreviousSocketDetection = true;
		}

		SocketHitBoxData[HitBoxChoice].PreviousSocketLocation = CurrentSocketLocation;

		FCollisionShape CollisionShape;

		switch (HitBoxData.ShapeType)
		{
		case ECustomCollisionShape::Box:
			CollisionShape = FCollisionShape::MakeBox(HitBoxData.BoxExtent);
			break;

		case ECustomCollisionShape::Sphere:
			CollisionShape = FCollisionShape::MakeSphere(HitBoxData.Radius);

			break;

		case ECustomCollisionShape::Capsule:
			CollisionShape = FCollisionShape::MakeCapsule(HitBoxData.Radius, HitBoxData.CapsuleHalfHeight);
			break;

		default:
			return; // no reason to continue if unknown shape
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (DebugCollision) {
			DrawDebugCollision(StartingCollisionDetectionLocation, CurrentSocketLocation, CollisionShape, HitBoxData, WorldRotation);
		}		
#endif

		// Store hit results
		TArray<FHitResult> HitResults;
		bool bHit = GetWorld()->SweepMultiByChannel(
			HitResults,     // Output results
			StartingCollisionDetectionLocation,  // Start position (converted from relative)
			CurrentSocketLocation,    // End position (same if not moving)
			WorldRotation,  // Rotation of the collision shape
			ECC_PhysicsBody,       // Collision Channel (adjust as needed)
			CollisionShape, // Collision shape
			SocketHitBoxData[HitBoxChoice].QueryParams    // Query parameters
		);

		if (!bHit) { return; }
		
		TSet<AActor*> AlreadyHitActors; // temporary local set for this frame

		for (const FHitResult& Hit : HitResults)
		{
			if (!Hit.GetActor()) { continue; }
			
			if (AlreadyHitActors.Contains(Hit.GetActor())) continue;

			AlreadyHitActors.Add(Hit.GetActor());

			SocketHitBoxData[HitBoxChoice].QueryParams.AddIgnoredActor(Hit.GetActor());
			OnNewHit(Hit, SocketHitBoxData[HitBoxChoice].currentHitBoxDamageInfo);
		}
	}
}

void UHitDetectionComponent::PerformCollisionDetectionForWeapon(int ChosenWeapon)
{
	if (AllWeaponsFullHitBoxes.IsEmpty()) return;

	for (const FHitBoxData& HitBoxData : AllWeaponsFullHitBoxes[ChosenWeapon].HitBoxesGroup.HitBoxes) {

		FVector CurrentSocketLocation;
		FQuat WorldRotation;

		if (OwnerMesh->DoesSocketExist(AllWeaponsFullHitBoxes[ChosenWeapon].HitBoxesGroup.SocketName)) {
			CurrentSocketLocation = OwnerMesh->GetSocketLocation(AllWeaponsFullHitBoxes[ChosenWeapon].HitBoxesGroup.SocketName);
			WorldRotation = OwnerMesh->GetSocketRotation(AllWeaponsFullHitBoxes[ChosenWeapon].HitBoxesGroup.SocketName).Quaternion();
		}
		else {
			CurrentSocketLocation = OwnerMesh->GetComponentLocation();
			WorldRotation = OwnerMesh->GetComponentQuat();
		}

		CurrentSocketLocation += WorldRotation.RotateVector(HitBoxData.RelativeLocation);
		WorldRotation = WorldRotation * HitBoxData.RelativeRotation.Quaternion();

		// set up starting location for sweep
		FVector StartingCollisionDetectionLocation; 
		if (AllWeaponsFullHitBoxes[ChosenWeapon].HitBoxesGroup.bUsePreviousSocketDetection) {
			StartingCollisionDetectionLocation = AllWeaponsFullHitBoxes[ChosenWeapon].HitBoxesGroup.PreviousSocketLocation;
		}
		else {
			StartingCollisionDetectionLocation = CurrentSocketLocation;
			AllWeaponsFullHitBoxes[ChosenWeapon].HitBoxesGroup.bUsePreviousSocketDetection = true;
		}

		// set up previous location for next detection
		AllWeaponsFullHitBoxes[ChosenWeapon].HitBoxesGroup.PreviousSocketLocation = CurrentSocketLocation;

		FCollisionShape CollisionShape;

		switch (HitBoxData.ShapeType)
		{
		case ECustomCollisionShape::Box:
			CollisionShape = FCollisionShape::MakeBox(HitBoxData.BoxExtent);
			break;

		case ECustomCollisionShape::Sphere:
			CollisionShape = FCollisionShape::MakeSphere(HitBoxData.Radius);
			break;

		case ECustomCollisionShape::Capsule:
			CollisionShape = FCollisionShape::MakeCapsule(HitBoxData.Radius, HitBoxData.CapsuleHalfHeight);
			break;

		default:
			return; // Unknown shape, do nothing
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (DebugCollision) {
			DrawDebugCollision(StartingCollisionDetectionLocation, CurrentSocketLocation, CollisionShape, HitBoxData, WorldRotation);
		}
#endif

		// Store hit results
		TArray<FHitResult> HitResults;

		// Perform the Sweep

		bool bHit = GetWorld()->SweepMultiByChannel(
			HitResults,     // Output results
			StartingCollisionDetectionLocation,  // Start position (converted from relative)
			CurrentSocketLocation,    // End position (same if not moving)
			WorldRotation,  // Rotation of the collision shape
			ECC_PhysicsBody,       // Collision Channel (adjust as needed)
			CollisionShape, // Collision shape
			AllWeaponsFullHitBoxes[ChosenWeapon].HitBoxesGroup.QueryParams     // Query parameters
		);

		// Process the results

		if (!bHit) { return; }
		
		TSet<AActor*> AlreadyHitActors; // temporary local set for this frame

		for (const FHitResult& Hit : HitResults)
		{
			if (!Hit.GetActor()) { continue; }
			
			if (AlreadyHitActors.Contains(Hit.GetActor())) continue;

			AlreadyHitActors.Add(Hit.GetActor());

			AllWeaponsFullHitBoxes[ChosenWeapon].HitBoxesGroup.QueryParams.AddIgnoredActor(Hit.GetActor());
			OnNewHit(Hit, AllWeaponsFullHitBoxes[ChosenWeapon].HitBoxesGroup.currentHitBoxDamageInfo);
		}
	}
}

void UHitDetectionComponent::OnNewHit(const FHitResult& DetectedHit, FDamageInfo HitDamage)
{
	UGameplayTagManagerComp* HittedTagManagerComp = nullptr;

	AActor* HitActor = DetectedHit.GetActor();

	if (HitActor && HitActor->GetClass()->ImplementsInterface(UTagManagerOwner::StaticClass()))
	{ 
		HittedTagManagerComp = ITagManagerOwner::Execute_GetTagManager(HitActor);
	}

	if (!HittedTagManagerComp || !OwnerTagComp) {return;}

	if (!OwnerTagComp->GetOwnerEnemiesFactionTags().HasTag(HittedTagManagerComp->GetOwnerFactionTag())) {
		return; // not an enemy
	}

	if (HittedTagManagerComp->GetTags().HasAny(TagsThatBlockHit)) {
		return;  // immune to hits
	}

	if (!HitActor->GetComponentByClass<UHealthComponent>()) return;

	if (UImpactComponent* hittedImpactComponent = HitActor->GetComponentByClass<UImpactComponent>()) {
		hittedImpactComponent->GotHit(DetectedHit, GetOwner(), HitDamage);
	}

	OnHitDetected.Broadcast(DetectedHit, HitDamage);
}

TArray<FString> UHitDetectionComponent::GetSocketNames() const
{
	TArray<FString> SocketNames;
	USkeletalMeshComponent* SkeletalMeshComp = nullptr;

	if (!GIsEditor)
	{
		if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
			SkeletalMeshComp = Owner->GetMesh();
	}
	else if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetOuter()))
	{
		if (ACharacter* CDO = Cast<ACharacter>(BPClass->GetDefaultObject()))
			SkeletalMeshComp = CDO->GetMesh();
	}

	if (SkeletalMeshComp)
	{
		for (const FName& SocketName : SkeletalMeshComp->GetAllSocketNames())
			SocketNames.Add(SocketName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSocketNames: No mesh found. Outer: %s"), *GetNameSafe(GetOuter()));
	}

	return SocketNames;
}

void UHitDetectionComponent::DrawDebugCollision(FVector DebugHitBoxStartLocation, FVector DebugHitBoxEndLocation, FCollisionShape DebugHitBoxShape, FHitBoxData DebugHitBoxData, FQuat DebugHitBoxRotation)
{
	// trying to make as accurate debug as I can so I decided to draw start then in the end and then line between them

	DrawDebugLine(GetWorld(), DebugHitBoxStartLocation, DebugHitBoxEndLocation, FColor::Blue, false, 0.2, 0, 1.5f);

	switch (DebugHitBoxData.ShapeType)
	{
	case ECustomCollisionShape::Box:
		DrawDebugBox(GetWorld(), DebugHitBoxStartLocation, DebugHitBoxData.BoxExtent, DebugHitBoxRotation, FColor::Red, false, 0.2);
		DrawDebugBox(GetWorld(), DebugHitBoxEndLocation, DebugHitBoxData.BoxExtent, DebugHitBoxRotation, FColor::Red, false, 0.2);

		break;
	case ECustomCollisionShape::Sphere:
		DrawDebugSphere(GetWorld(), DebugHitBoxStartLocation, DebugHitBoxData.Radius, 12, FColor::Red, false, 0.2);
		DrawDebugSphere(GetWorld(), DebugHitBoxEndLocation, DebugHitBoxData.Radius, 12, FColor::Red, false, 0.2);

		break;
	case ECustomCollisionShape::Capsule:
		DrawDebugCapsule(GetWorld(), DebugHitBoxStartLocation, DebugHitBoxData.CapsuleHalfHeight, DebugHitBoxData.Radius, DebugHitBoxRotation, FColor::Red, false, 0.2);
		DrawDebugCapsule(GetWorld(), DebugHitBoxEndLocation, DebugHitBoxData.CapsuleHalfHeight, DebugHitBoxData.Radius, DebugHitBoxRotation, FColor::Red, false, 0.2);

		break;

	default:
		break;
	}
}
 
TArray<AActor*> UHitDetectionComponent::GetActorsInFrontOfOwner()
{
	TArray<AActor*> OutActors;

	AActor* Owner = GetOwner();
	if (!Owner)
		return OutActors;

	UWorld* World = GetWorld();
	if (!World)
		return OutActors;

	// ---- CONFIG ----
	const float ForwardOffset = 150.f;
	const FVector BoxExtent = FVector(100.f, 100.f, 200.f);
	const float TraceExtraDistance = 50.f;

	// ---- TRANSFORM ----
	FVector OwnerLocation = Owner->GetActorLocation();
	FVector ForwardVector = Owner->GetActorForwardVector();

	FVector BoxCenter = OwnerLocation + ForwardVector * ForwardOffset;
	FQuat BoxRotation = Owner->GetActorQuat();

	// ---- SWEEP (instead of overlap) ----
	FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);

	FCollisionQueryParams SweepParams;
	SweepParams.AddIgnoredActor(Owner);
	SweepParams.bReturnPhysicalMaterial = false;

	//DrawDebugBox(World, BoxCenter, BoxExtent, BoxRotation, FColor::Green, false, 2.f);

	TArray<FHitResult> Hits;
	bool bHit = World->SweepMultiByChannel(
		Hits,
		BoxCenter,          // Start
		BoxCenter,          // End (same = static volume)
		BoxRotation,
		ECC_Pawn,
		BoxShape,
		SweepParams
	);

	if (!bHit)
		return OutActors;

	// ---- LINE TRACE CONFIG ----
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(Owner);

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
			continue;

		USkeletalMeshComponent* MeshComp =
			Cast<USkeletalMeshComponent>(Hit.GetComponent());

		if (!MeshComp)
			continue;

		// Use socket/bone from the hit result if available
		FName HitSocketName = Hit.BoneName;
		FVector SocketLocation;

		if (HitSocketName != NAME_None && MeshComp->DoesSocketExist(HitSocketName))
		{
			SocketLocation = MeshComp->GetSocketLocation(HitSocketName);
		}
		else
		{
			// Fallback if bone/socket is missing
			SocketLocation = Hit.ImpactPoint;
		}

		// ---- LINE TRACE TO SOCKET ----
		FVector TraceDirection = (SocketLocation - OwnerLocation).GetSafeNormal();
		FVector TraceEnd = SocketLocation + TraceDirection * TraceExtraDistance;

		FHitResult LineHit;
		bool bBlocked = World->LineTraceSingleByChannel(
			LineHit,
			OwnerLocation,
			TraceEnd,
			ECC_Visibility,
			TraceParams
		);

		OutActors.AddUnique(HitActor);
	}

	return OutActors;
}
