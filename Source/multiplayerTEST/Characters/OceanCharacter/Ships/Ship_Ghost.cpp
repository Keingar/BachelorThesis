#include "Ship_Ghost.h"
#include "Components/StaticMeshComponent.h"

AShip_Ghost::AShip_Ghost()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Static);
	SetRootComponent(Root);

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	ShipMesh->SetupAttachment(Root);

	ShipMesh->SetMobility(EComponentMobility::Static);
	ShipMesh->SetCollisionProfileName(TEXT("BlockAll"));

	bReplicates = false;
}

void AShip_Ghost::BeginPlay()
{
	Super::BeginPlay();
}