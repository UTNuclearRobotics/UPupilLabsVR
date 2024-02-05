// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Chifor Tudor
// Updated by: Christina Petlowany, The University of Texas at Austin

#include "MyTestPupilActor.h"
#include <zmq.hpp>

AMyTestPupilActor::AMyTestPupilActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMyTestPupilActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("PupilActor>>>>BeginPlay"));

	PupilComm = std::make_unique<FPupilMsgWorker>();
	// PupilComm = FPupilMsgWorker::StartListening();

	PupilComm->OnNewData().AddUObject(this, &AMyTestPupilActor::OnNewPupilData);

	World = GetWorld();
}

// Called every frame
void AMyTestPupilActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!PupilComm->CanGaze())
	{
		PupilComm->UpdateCalibration();
	}
}

void AMyTestPupilActor::OnNewPupilData(GazeStruct* GazeStructure)
{
	this->ReceivedGazeStructure = GazeStructure;
	Location_r = PupilComm->GetLocation_R();
	Rotation_r = PupilComm->GetRotation_R();
	Location_l = PupilComm->GetLocation_L();
	Rotation_l = PupilComm->GetRotation_L();
	Eigen::Quaternionf q_1(Rotation_r);
	Eigen::Quaternionf q_2(Rotation_l);
	q_r = q_1;
	q_l = q_2;
	SendData();
}

void AMyTestPupilActor::SendData()
{
	NewPupilDataEvent.Broadcast(ReceivedGazeStructure);
}

// Change to delegate
// Create a local copy of the latest data received
// Avoid dynamic memory allocation
FUEStruct AMyTestPupilActor::PupilData()
{
	//Eigen::Vector3f Location_r = PupilComm->GetLocation_R();
	//Eigen::Matrix3f Rotation_r = PupilComm->GetRotation_R();
	//Eigen::Vector3f Location_l = PupilComm->GetLocation_L();
	//Eigen::Matrix3f Rotation_l = PupilComm->GetRotation_L();
	//Eigen::Quaternionf q_r(Rotation_r);
	//Eigen::Quaternionf q_l(Rotation_l);

	FUEStruct pupilStruct;

	if (PupilComm->CanGaze())
	{
		if (ReceivedGazeStructure->topic == "gaze.3d.01.")
		{
			pupilStruct.confidence = ReceivedGazeStructure->confidence;
			pupilStruct.pupil_d_r = ReceivedGazeStructure->base_data.pupil1.diameter;
			pupilStruct.pupil_d_l = ReceivedGazeStructure->base_data.pupil2.diameter;
			for (std::map<std::string, vector_3d>::iterator it = ReceivedGazeStructure->gaze_normals_3d.begin(); it != ReceivedGazeStructure->gaze_normals_3d.end(); ++it)
			{
				std::string eye_d = it->first;
				vector_3d eye_vec = it->second;
				if (it->first == "0")
				{
					pupilStruct.eye_loc_r.X = Location_r.x();
					pupilStruct.eye_loc_r.Y = Location_r.y();
					pupilStruct.eye_loc_r.Z = Location_r.z();
					pupilStruct.gaze_dir_r.X = it->second.x;
					pupilStruct.gaze_dir_r.Y = it->second.y;
					pupilStruct.gaze_dir_r.Z = it->second.z;
					pupilStruct.gaze_rot_r = FQuat(q_r.x(), q_r.y(), q_r.z(), q_r.w());
				}
				else if (it->first == "1")
				{
					pupilStruct.eye_loc_l.X = Location_l.x();
					pupilStruct.eye_loc_l.Y = Location_l.y();
					pupilStruct.eye_loc_l.Z = Location_l.z();
					pupilStruct.gaze_dir_l.X = it->second.x;
					pupilStruct.gaze_dir_l.Y = it->second.y;
					pupilStruct.gaze_dir_l.Z = it->second.z;
					pupilStruct.gaze_rot_l = FQuat(q_l.x(), q_l.y(), q_l.z(), q_l.w());
				}
			}
		}
	}
	return pupilStruct;
}

bool AMyTestPupilActor::CanGaze()
{
	bool can_gaze = PupilComm->CanGaze();
	return can_gaze;
}

void AMyTestPupilActor::StartCalibration()
{
	//SPAWN PAWN
	FVector SpawnLocation(1000, 1000, 1000);
	FRotator SpawnRotation(0.0f, 0.0f, 0.0f);
	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();

	ACalibrationMarker* CalibrationMarker = GetWorld()->SpawnActor<ACalibrationMarker>(ACalibrationMarker::StaticClass(), SpawnLocation, SpawnRotation, SpawnParameters);

	 PupilComm->SetCalibrationMarker(CalibrationMarker, GetWorld());
}

void AMyTestPupilActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogTemp, Warning, TEXT("PupilActor>>>>EndPlay"));
	Super::EndPlay(EndPlayReason);
}