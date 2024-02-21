// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Chifor Tudor
// Updated by: Christina Petlowany, The University of Texas at Austin

#include "FPupilMsgWorker.h"


/**
 * \brief Thread which connects and receive data from a certain @PupilTopic from Pupil Serice.
 * Start- and StopListening are all what is needed to use this Thread.
 */
FPupilMsgWorker::FPupilMsgWorker()
{
	Thread = FRunnableThread::Create(this, TEXT("PupilMsgWorker"), TPri_BelowNormal);
}

/**
 * \brief Deconstructor that waits for completion of the Task and then deletes the Thread.
 */
FPupilMsgWorker::~FPupilMsgWorker()
{
	UE_LOG(LogTemp, Warning, TEXT("Destructor called"));
	if (Thread) {
		Stop();
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}
}

/**
 * \brief Initializes the context and socket to connect to Pupil Service.
 *	Starts Run() if successful.
 * \return true if the Initialization was  successful
 */
bool FPupilMsgWorker::Init()
{
	bRunning = true;
	return true;
}

/**
* \brief Constantly retrives data from Pupil Service if it has been successfuly initialized & nobody has stopped the Thread.
* \return A number indicating where it has failed TODO
*/
uint32 FPupilMsgWorker::Run()
{
	FPlatformProcess::Sleep(0.5);
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);

	while (bRunning)
	{
		//FPlatformProcess::Sleep(0.01);
		// ReceivedGazeStructure = PupilHelper.GetGazeStructure();
		// UE_LOG(LogTemp, Warning, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
		// ReceivedGazeStructure = PupilHelper.GetGazeStructure();
		// FDateTime CurrentUETimestamp = FDateTime::UtcNow();
		// FString write_data = FString::FromInt(CurrentUETimestamp.GetMinute() * 60 + CurrentUETimestamp.GetSecond()) + "." + FString::FromInt(CurrentUETimestamp.GetMillisecond()) + "," + PupilHelper.GetWriteData() + "\n";
		// FFileHelper::SaveStringToFile(write_data, *(FPaths::ProjectConfigDir() + UTF8TEXT("SaveFileTestPL")), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
		can_gaze = PupilHelper.CanGaze();
		if(can_gaze){
		ReceivedGazeStructure = PupilHelper.GetGazeStructure();
		Rotation_r = PupilHelper.GetRotation_R();
		Location_r = PupilHelper.GetLocation_R();
		//FDateTime CurrentUETimestamp = FDateTime::UtcNow();
		//FString write_data = FString::FromInt(CurrentUETimestamp.GetMinute() * 60 + CurrentUETimestamp.GetSecond()) + "." + FString::FromInt(CurrentUETimestamp.GetMillisecond()) + "," + PupilHelper.GetWriteData() + "\n";
		//FFileHelper::SaveStringToFile(write_data, *(FPaths::ProjectConfigDir() + UTF8TEXT("SaveFileTestPL")), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
		//GazeStruct ReceivedGazeStruct;
		NewPupilDataEvent.Broadcast(&ReceivedGazeStructure);
		}
	}
	return 0;
}

void FPupilMsgWorker::Stop()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop Called"));
	bRunning = false;
}

void FPupilMsgWorker::SetCalibrationMarker(ACalibrationMarker* MarkerRef, UWorld* World, FVector Pos1, FVector Pos2, FVector Pos3, FVector Pos4, FVector Pos5)
{
	PupilHelper.SetCalibrationMarker(MarkerRef, World, Pos1, Pos2, Pos3, Pos4, Pos5);
}

void OnNewData()
{
	
}

void FPupilMsgWorker::UpdateCalibration()
{
	PupilHelper.UpdateCustomCalibration();
}

Eigen::Matrix3f FPupilMsgWorker::GetRotation_R()
{
	return Rotation_r;
}

Eigen::Matrix3f FPupilMsgWorker::GetRotation_L()
{
	return Rotation_l;
}

Eigen::Vector3f FPupilMsgWorker::GetLocation_R()
{
	return Location_r;
}

Eigen::Vector3f FPupilMsgWorker::GetLocation_L()
{
	return Location_l;
}

bool FPupilMsgWorker::CanGaze()
{
	return can_gaze;
}