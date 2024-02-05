// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class UPupilLabsVR : ModuleRules
{

	private string ThirdPartyPath
	{
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/")); }
	}

	public void LoadEigen()
	{
		// Create Eigen Path 
		string EigenPath = Path.Combine(ThirdPartyPath, "Eigen");

		//Add Include path 
		PublicIncludePaths.AddRange(new string[] { Path.Combine(EigenPath, "includes") });
	}

	public UPupilLabsVR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnableUndefinedIdentifierWarnings = false;

		LoadEigen();

        PublicIncludePaths.AddRange(
            new string[] {
				Path.Combine(PluginDirectory,"Source/UPupilLabsVR/public"),
				// ... add public include paths required here ...
			}
            );


        PrivateIncludePaths.AddRange(
            new string[] {
                "UPupilLabsVR/Private",
				// ... add other private include paths required here ...
			}
            );


        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine",
                "libzmq",
                "msgpack",
				"EyeTracker",
                "Projects"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
