// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Chifor Tudor
// Updated by: Christina Petlowany, The University of Texas at Austin

#include "FPupilMsgWorker.h"

FPupilMsgWorker* FPupilMsgWorker::Instance = nullptr;

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
	if (Thread != nullptr) {
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
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
	return true; //TODO BUILD A ERROR BASED LOG. FALSE IF THE INITIALISATION FAILED//O Metoda ce sa returneze un numar de Eroare sau ceva similar
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
		UE_LOG(LogTemp, Warning, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
		ReceivedGazeStructure = PupilHelper.GetGazeStructure();
		FDateTime CurrentUETimestamp = FDateTime::UtcNow();
		FString write_data = FString::FromInt(CurrentUETimestamp.GetMinute() * 60 + CurrentUETimestamp.GetSecond()) + "." + FString::FromInt(CurrentUETimestamp.GetMillisecond()) + "," + PupilHelper.GetWriteData() + "\n";
		FFileHelper::SaveStringToFile(write_data, *(FPaths::ProjectConfigDir() + UTF8TEXT("SaveFileTestPL")), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
		if(PupilHelper.CanGaze()){
		ReceivedGazeStructure = PupilHelper.GetGazeStructure();
		Rotation_r = PupilHelper.GetRotation_R();
		Location_r = PupilHelper.GetLocation_R();
		can_gaze = PupilHelper.CanGaze();
		//FDateTime CurrentUETimestamp = FDateTime::UtcNow();
		//FString write_data = FString::FromInt(CurrentUETimestamp.GetMinute() * 60 + CurrentUETimestamp.GetSecond()) + "." + FString::FromInt(CurrentUETimestamp.GetMillisecond()) + "," + PupilHelper.GetWriteData() + "\n";
		//FFileHelper::SaveStringToFile(write_data, *(FPaths::ProjectConfigDir() + UTF8TEXT("SaveFileTestPL")), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
		//GazeStruct ReceivedGazeStruct;
		NewPupilDataEvent.Broadcast(&ReceivedGazeStructure);
		}
	}
	return 1;
}

void FPupilMsgWorker::Stop()
{
	Instance->bRunning = false;
}

void FPupilMsgWorker::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FPupilMsgWorker::Shutdown()
{
	if (Instance)
	{
		Instance->EnsureCompletion();
		delete Instance;
		Instance = nullptr;
	}
}

/**
* \brief This method checks if possible and then starts a Thread and Initializes it.
* The Initialization also starts the Thread by calling the Run() method.
* \return Instance of the object containing the Worker Thread
*/
FPupilMsgWorker* FPupilMsgWorker::StartListening()
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Instance && FPlatformProcess::SupportsMultithreading())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
		Instance = new FPupilMsgWorker();
		//Instance->bSuccessfulyInit = Instance->Init();
		Instance->bRunning = true;
	}
	return Instance;
}
/*
mayer's singleton
widget* getInstance()
{
static widget my_only_one;
return &my_only_one;
}
/
/**
 * \brief Waits and Stops the Thread in the correct way.Stops the process and then destroys the Thread.
 */
void FPupilMsgWorker::StopListening()
{
	if (Thread != nullptr) {
		Instance->Shutdown();
		delete Thread;
		Thread = nullptr;
	}
}

void FPupilMsgWorker::SetCalibrationMarker(ACalibrationMarker* MarkerRef, UWorld* World)
{
	PupilHelper.SetCalibrationMarker(MarkerRef, World);
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