#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ship_Ghost.generated.h"

class UStaticMeshComponent;

UCLASS()
class MULTIPLAYERTEST_API AShip_Ghost : public AActor
{
	GENERATED_BODY()

public:
	AShip_Ghost();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* ShipMesh;

protected:
	virtual void BeginPlay() override;
};