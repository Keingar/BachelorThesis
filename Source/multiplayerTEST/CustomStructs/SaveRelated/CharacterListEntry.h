#pragma once

#include "CoreMinimal.h"
#include "CharacterListEntry.generated.h"

USTRUCT(BlueprintType)
struct FCharacterListEntry
{
    GENERATED_BODY()

public:
    // Stable ID used for save slot
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    FString CharacterID; // GUID string

    // Display name shown in UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    FText DisplayName;
};
