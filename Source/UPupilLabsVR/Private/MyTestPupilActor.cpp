// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Chifor Tudor

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
	//SPAWN PAWN
	FVector SpawnLocation(1000, 1000, 1000);
	FRotator SpawnRotation(0.0f, 0.0f, 0.0f);
	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();
	// AAPupilLabsVisualMarkersPawn* CalibrationScenePawn = GetWorld()->SpawnActor<AAPupilLabsVisualMarkersPawn>(AAPupilLabsVisualMarkersPawn::StaticClass(), SpawnLocation, SpawnRotation, SpawnParameters);
	//SPAWN PAWN

	ACalibrationMarker* CalibrationMarker = GetWorld()->SpawnActor<ACalibrationMarker>(ACalibrationMarker::StaticClass(), SpawnLocation, SpawnRotation, SpawnParameters);

	PupilComm = FPupilMsgWorker::StartListening();

	PupilComm->SetCalibrationMarker(CalibrationMarker, GetWorld());

	// PupilComm->SetVisualsReference(CalibrationScenePawn);
	PupilComm->OnNewData().AddUObject(this, &AMyTestPupilActor::OnNewPupilData);
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);\

	World = GetWorld();
}
// Called every frame
void AMyTestPupilActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PupilComm->UpdateCalibration();
	// PerformRaycast(World);
}

void AMyTestPupilActor::OnNewPupilData(GazeStruct* GazeStructure)
{
	this->ReceivedGazeStructure = GazeStructure;
	canRayCast = true;
	// UE_LOG(LogTemp, Warning, TEXT("[%s][%d] Norm Data : %f"), TEXT(__FUNCTION__), __LINE__, this->ReceivedGazeStructure->base_data.pupil.ellipse.center.x);
	// UE_LOG(LogTemp, Warning, TEXT("[%s][%d] DirX : %f"), TEXT(__FUNCTION__), __LINE__, this->ReceivedGazeStructure->gaze_normal_3d.x);
	// UE_LOG(LogTemp, Warning, TEXT("[%s][%d] DirY : %f"), TEXT(__FUNCTION__), __LINE__, this->ReceivedGazeStructure->gaze_normal_3d.y);
	// UE_LOG(LogTemp, Warning, TEXT("[%s][%d] DirZ : %f"), TEXT(__FUNCTION__), __LINE__, this->ReceivedGazeStructure->gaze_normal_3d.z);
	// UE_LOG(LogTemp, Warning, TEXT("[%s][%d] Text : %f"), TEXT(__FUNCTION__), __LINE__, this->ReceivedGazeStructure->confidence);
	// UE_LOG(LogTemp, Warning, TEXT("[%s][%d] eye_center_x : %f"), TEXT(__FUNCTION__), __LINE__, this->ReceivedGazeStructure->eye_center_3d.x);
}

void AMyTestPupilActor::PerformRaycast(UWorld* CurrentWorld)
{
	//APlayerController* UPupilPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (canRayCast)
	{
		if (ReceivedGazeStructure->confidence > 0.6 && ReceivedGazeStructure->topic == "gaze.3d.01.")
		{
			if (ReceivedGazeStructure->gaze_normals_3d.begin()->first == "0")
			{
				APlayerCameraManager* camManager = CurrentWorld->GetFirstPlayerController()->PlayerCameraManager;
				FVector HMDposition = camManager->GetCameraLocation();
				FRotator HMDorientation = camManager->GetCameraRotation();
				Eigen::Matrix3f Rotation = PupilComm->GetRotation();
				Eigen::Vector3f Location = PupilComm->GetLocation();
				FVector TraceStart = FVector(Location.x(), Location.y(), Location.z()) + HMDposition;
				TraceStart = HMDposition;
				Eigen::Vector3f TraceVec = Rotation * Eigen::Vector3f(ReceivedGazeStructure->gaze_normals_3d.begin()->second.x, ReceivedGazeStructure->gaze_normals_3d.begin()->second.y, ReceivedGazeStructure->gaze_normals_3d.begin()->second.z).normalized();
				FVector TraceDir = FVector(TraceVec.x(), -TraceVec.y(), TraceVec.z());
				// UE_LOG(LogTemp, Warning, TEXT("tracedir is %s"), *TraceDir.ToString());
				// FVector TraceEnd = TraceStart + 50 * HMDorientation.RotateVector(TraceDir);
				FVector TraceEnd = TraceStart + 1 * HMDorientation.Vector();
				UE_LOG(LogTemp, Warning, TEXT("tracedir is %s"), *HMDorientation.RotateVector(TraceDir).ToString());
				FHitResult* HitResult = new FHitResult(ForceInit);
				FCollisionQueryParams* TraceParams = new FCollisionQueryParams();
				DrawDebugLine(CurrentWorld, TraceStart, TraceEnd, FColor(255, 0, 0), true, 0.3, 255, 10.0);
			}
		}
	}

	//if (GetWorld()->LineTraceSingleByChannel(*HitResult, TraceStart, TraceEnd, ECC_Visibility, *TraceParams))
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
	//	//	UE_LOG(LogTemp, Warning, TEXT("[%s][%d]RAYTRACE XXX : %f"), TEXT(__FUNCTION__), __LINE__, XGaze);
	//	//	UE_LOG(LogTemp, Warning, TEXT("[%s][%d]RAYTRACE YYY : %f"), TEXT(__FUNCTION__), __LINE__, YGaze);
	//	FVector_NetQuantize var = HitResult->ImpactPoint;
	//	FVector HitPointLocation = var;
	//	DrawDebugPoint(GetWorld(), TraceEnd, 20, FColor(0, 255, 127), false, 1.03);

	//	DrawDebugPoint(GetWorld(), HitPointLocation, 20, FColor(0, 0, 238), false, 0.03);
	//	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor(238, 0, 238), true);
	//}
}

////MOUSE TEST
//FMatrix CameraViewMatrix; //Todo maybe this is the way
//FMatrix CameraProjectionMatrix;
//FMatrix inv = Inverse(CameraProjectionMatrix * CameraViewMatrix);
////

FUEStruct AMyTestPupilActor::PupilData()
{
	Eigen::Vector3f Location = PupilComm->GetLocation();
	Eigen::Matrix3f Rotation = PupilComm->GetRotation();
	Eigen::Quaternionf q(Rotation);
	FRotator UERot = FRotator(FQuat(q.x(), q.y(), q.z(), q.w()));
	FUEStruct pupilStruct;
	if (canRayCast)
	{
		if (ReceivedGazeStructure->confidence > 0.6 && ReceivedGazeStructure->topic == "gaze.3d.01.")
		{
			if (ReceivedGazeStructure->gaze_normals_3d.begin()->first == "0")
			{
				pupilStruct.eye_loc.X = Location.x();
				pupilStruct.eye_loc.Y = Location.y();
				pupilStruct.eye_loc.Z = Location.z();
				pupilStruct.gaze_dir.X = ReceivedGazeStructure->gaze_normals_3d.begin()->second.x;
				pupilStruct.gaze_dir.Y = -ReceivedGazeStructure->gaze_normals_3d.begin()->second.y;
				pupilStruct.gaze_dir.Z = ReceivedGazeStructure->gaze_normals_3d.begin()->second.z;
				pupilStruct.gaze_rot = UERot;
			}

		}
	}
//
	return pupilStruct;
}
