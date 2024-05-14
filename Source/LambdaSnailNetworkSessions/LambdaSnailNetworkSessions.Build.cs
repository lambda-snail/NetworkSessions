// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LambdaSnailNetworkSessions : ModuleRules
{
	public LambdaSnailNetworkSessions(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"OnlineSubsystem",
			"OnlineSubsystemSteam",
			"UMG",
			"Slate",
			"SlateCore"
		});
			
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine"
		});
	}
}
