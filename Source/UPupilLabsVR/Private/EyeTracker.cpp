// Author: Christina Petlowany, The University of Texas at Austin

#include "EyeTracker.h"

AEyeTracker::AEyeTracker()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEyeTracker::BeginPlay()
{
	Super::BeginPlay();
    World = GetWorld();
}

// Called every frame
void AEyeTracker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (UEyeTrackerFunctionLibrary::IsEyeTrackerConnected())
    {
        FEyeTrackerGazeData GazeData;
        UEyeTrackerFunctionLibrary::GetGazeData(GazeData);
        APlayerCameraManager* camManager = World->GetFirstPlayerController()->PlayerCameraManager;
        FVector HMDposition = camManager->GetCameraLocation();
        FQuat HMDorientation = camManager->GetCameraRotation().Quaternion();
        // GazeData.ConfidenceValue;
        // GazeData.GazeDirection;
        // GazeData.GazeOrigin
        // FDateTime CurrentUETimestamp = FDateTime::UtcNow();
        // FString SaveText = FString::FromInt(CurrentUETimestamp.GetMinute() * 60 + CurrentUETimestamp.GetSecond()) + "." + FString::FromInt(CurrentUETimestamp.GetMillisecond()) + "," + FString::SanitizeFloat(GazeData.ConfidenceValue) + "," + GazeData.GazeDirection.ToString() + "," + GazeData.GazeOrigin.ToString() + "," + HMDposition.ToString() + "," + HMDorientation.ToString() + "\n";
        // FFileHelper::SaveStringToFile(SaveText, *(FPaths::ProjectConfigDir() + UTF8TEXT("SaveFileTestHL")), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
    }
}