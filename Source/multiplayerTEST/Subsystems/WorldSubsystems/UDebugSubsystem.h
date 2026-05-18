#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UDebugSubsystem.generated.h"


UCLASS()
class MULTIPLAYERTEST_API UUDebugSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

private:
	void PrintActiveTimers();

	FTimerHandle DebugTimerHandle;
};
