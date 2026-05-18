#include "UDebugSubsystem.h"

#if !UE_BUILD_SHIPPING

#include "Engine/World.h"
#include "TimerManager.h"

void UUDebugSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning,
		TEXT("TimerDebugSubsystem CREATED for world: %s"),
		*GetWorld()->GetName());

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DebugTimerHandle,
			this,
			&UUDebugSubsystem::PrintActiveTimers,
			0.1f,   // every 0.1 sec
			true    // looping
		);
	}
}

bool UUDebugSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	if (!World)
	{
		return false;
	}

	if (World->WorldType != EWorldType::Game &&
		World->WorldType != EWorldType::PIE)
	{
		return false;
	}

	// Block menu / frontend maps
	const FString MapName = World->GetMapName();

	if (MapName.Contains(TEXT("StartMenu")))
	{
		return false;
	}

	return false; 
}

void UUDebugSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DebugTimerHandle);
	}

	Super::Deinitialize();
}

void UUDebugSubsystem::PrintActiveTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ListTimers();
	}
}

#else 

void UUDebugSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

bool UUDebugSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return false;
}

void UUDebugSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

#endif 
