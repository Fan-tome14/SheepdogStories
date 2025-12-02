// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SheepdogStories : ModuleRules
{
    public SheepdogStories(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "MassEntity",
            "MassCommon",
            "MassSpawner",
            "MassGameplayDebug",
            "NavigationSystem",
            "AIModule"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}