// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class multiplayerTEST : ModuleRules
{
	public multiplayerTEST(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "UMG", "AIModule", "RenderCore", "Water", "Niagara" });

        PrivateDependencyModuleNames.AddRange(new string[] {"GameplayTags", "NavigationSystem"});



        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}
		
