#include "ObjectRegistryComponent.h"
#include "multiplayerTEST/Subsystems/WorldSubsystems/RegistrySubsystem.h"

void UObjectRegistryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ObjectID.IsNone()) {
		UE_LOG(LogTemp, Error, TEXT("ObjectID is not set for %s in %s"), *GetOwner()->GetName(), *GetName());
		return;
	}

	if (URegistrySubsystem* Registry =
		GetWorld()->GetSubsystem<URegistrySubsystem>())
	{
		Registry->RegisterObject(ObjectID, GetOwner());
	}
}

void UObjectRegistryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (URegistrySubsystem* Registry =
		GetWorld()->GetSubsystem<URegistrySubsystem>())
	{
		Registry->UnregisterObject(ObjectID);
	}

	Super::EndPlay(EndPlayReason);
}
