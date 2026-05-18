// Copyright Epic Games, Inc. All Rights Reserved.

#include "multiplayerTEST.h"
#include "ShaderCore.h"

#include "Modules/ModuleManager.h"

void FOceanTutorialModule::StartupModule()
{
    // creating additional virtual path for water niagara simulation so paths don't break for different PC
    FString ShaderDirectory = FPaths::Combine(
        FPaths::GameSourceDir(),
        TEXT("multiplayerTEST/OceanWater/Shaders")
    );

    AddShaderSourceDirectoryMapping(TEXT("/Project"), ShaderDirectory);

    UE_LOG(LogTemp, Log, TEXT("Mapped /Project -> %s"), *ShaderDirectory);
}

IMPLEMENT_PRIMARY_GAME_MODULE( FOceanTutorialModule, multiplayerTEST, "multiplayerTEST" );
 