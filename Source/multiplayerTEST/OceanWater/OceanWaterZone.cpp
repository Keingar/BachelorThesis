#include "OceanWaterZone.h"

#include "GameFramework/GameState.h"
#include "WaterMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "WaterSubsystem.h"

AOceanWaterZone::AOceanWaterZone(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    FFTCalculator.Initialize();

    PrimaryActorTick.bCanEverTick = true;
}

void AOceanWaterZone::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    FFTCalculator.Calculate(GetWorld()->GetGameState());
    FFTCalculator.ShowDebugDisplacementPoints(GetWorld(), GetActorLocation());
}

bool AOceanWaterZone::ShouldTickIfViewportsOnly() const
{
    if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor)
    {
        return true;
    }
    return false;
}