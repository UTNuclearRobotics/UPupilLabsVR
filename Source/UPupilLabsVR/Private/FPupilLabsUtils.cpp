// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Chifor Tudor
// Updated by: Christina Petlowany, The University of Texas at Austin

#include "FPupilLabsUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include <sstream>
#include <Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>

FPupilLabsUtils::FPupilLabsUtils()
{
	// Connect to Pupil Labs
	zmq::socket_t ReqSocket = ConnectToZmqPupilPublisher(Port);
	SubSocket = ConnectToSubport(&ReqSocket, PupilTopic);
	// Synchronize time
	SynchronizePupilServiceTimestamp();
	
   	ReqSocket.close();

	Eigen::Matrix3f test_mat;
	test_mat << 0, 0, 1,
		-1, 0, 0,
		0, -1, 0;
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, TEXT("Print R"));
	UE_LOG(LogTemp, Warning, TEXT("Row 1 is: %f, %f, %f"), test_mat.coeff(0, 0), test_mat.coeff(0, 1), test_mat.coeff(0, 2)); // DO NOT DELETE (SAVE FOR REFERENCE)
	UE_LOG(LogTemp, Warning, TEXT("Row 2 is: %f, %f, %f"), test_mat.coeff(1, 0), test_mat.coeff(1, 1), test_mat.coeff(1, 2));
	UE_LOG(LogTemp, Warning, TEXT("Row 3 is: %f, %f, %f"), test_mat.coeff(2, 0), test_mat.coeff(2, 1), test_mat.coeff(2, 2));
	Eigen::Quaternionf q(test_mat);
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, TEXT("Print quat"));
	UE_LOG(LogTemp, Warning, TEXT("quat is: %f, %f, %f, %f"), q.x(), q.y(), q.z(), q.w()); // DO NOT DELETE (SAVE FOR REFERENCE)
	FRotator UERot = FRotator(FQuat(q.x(), q.y(), q.z(), q.w()));
	UE_LOG(LogTemp, Warning, TEXT("rotat: %f %f %f"), UERot.Pitch, UERot.Roll, UERot.Yaw);
	Eigen::Vector3f euler = test_mat.eulerAngles(2, 1, 0);
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, TEXT("Print euler"));
	UE_LOG(LogTemp, Warning, TEXT("euler is: %f, %f, %f"), euler.x(), euler.y(), euler.z()); // DO NOT DELETE (SAVE FOR REFERENCE)
}

FPupilLabsUtils::~FPupilLabsUtils()
{
	// Close the subsocket
	ZmqContext->close();
	if (!bSubSocketClosed)
	{
		SubSocket->close();
	}
	ZmqContext = nullptr;
	SubSocket = nullptr;
}

zmq::socket_t FPupilLabsUtils::ConnectToZmqPupilPublisher(const std::string ReqPort) {
	// Connect to PL ZMQ socket with given address and req port, return req socket
	ZmqContext = new zmq::context_t(1);
	std::string ConnAddr = Addr + ReqPort;
	zmq::socket_t ReqSocket(*ZmqContext, ZMQ_REQ);
	ReqSocket.connect(ConnAddr);

	return ReqSocket;
}

zmq::socket_t* FPupilLabsUtils::ConnectToSubport(zmq::socket_t *ReqSocket,const std::string Topic)
{
	// Take req socket and open connection to Pupil Service by binding a SubSocket
	std::string SendSubPort = u8"SUB_PORT";
	zmq::message_t subport_request(SendSubPort.size());
	memcpy(subport_request.data(), SendSubPort.c_str(), SendSubPort.length());
	ReqSocket->send(subport_request);
	// Subscriber socket
	std::string SubPortAddr = Addr + ReceiveSubPort(ReqSocket);
	zmq::socket_t* SubSocketTemp = new zmq::socket_t(*ZmqContext, ZMQ_SUB);
	SubSocketTemp->connect(SubPortAddr);
	SubSocketTemp->setsockopt(ZMQ_SUBSCRIBE, Topic.c_str(), Topic.length());
	bSubSocketClosed = false;

	return SubSocketTemp;
}

GazeStruct FPupilLabsUtils::ConvertMsgPackToGazeStruct(zmq::message_t info)
{
	// Convert Pupil data to the Gaze Struct
	char* payload = static_cast<char*>(info.data());
	msgpack::object_handle oh = msgpack::unpack(payload, info.size());
	msgpack::object deserialized = oh.get();
	GazeStruct ReceivedGazeStruct;

	std::stringstream ss;
	ss << deserialized;
	std::string demo = ss.str();
	deserialized.convert(ReceivedGazeStruct);
	FString SaveText = UTF8_TO_TCHAR(demo.c_str());
	// SaveData(SaveText);
	return ReceivedGazeStruct;
}

std::string FPupilLabsUtils::ReceiveSubPort(zmq::socket_t *ReqSocket)
{
	zmq::message_t Reply;
	ReqSocket->recv(&Reply);
	std::string  SubportReply = std::string(static_cast<char*>(Reply.data()), Reply.size());
	LogReply(SubportReply);

	return SubportReply;
}

void FPupilLabsUtils::CloseSubSocket()
{
	bSubSocketClosed = true;
	SubSocket->close();
}

void FPupilLabsUtils::LogReply(std::string SubportReply)
{
	FString PortRequest(SubportReply.c_str());
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, *PortRequest);
}

void FPupilLabsUtils::SynchronizePupilServiceTimestamp()
{
	zmq::socket_t TimeReqSocket = ConnectToZmqPupilPublisher(Port);
	
	float CurrentUETimestamp = FPlatformTime::Seconds();
	std::string SendCurrentUETimeStamp = u8"T " + std::to_string(CurrentUETimestamp);
	FString SendTimestamp(SendCurrentUETimeStamp.c_str());
	UE_LOG(LogTemp, Warning, TEXT("Current TimeStamp %s "), *SendTimestamp);

	zmq::message_t TimestampSendRequest(SendCurrentUETimeStamp.length());
	memcpy(TimestampSendRequest.data(), SendCurrentUETimeStamp.c_str(), SendCurrentUETimeStamp.length());
	TimeReqSocket.send(TimestampSendRequest);

	zmq::message_t Reply;
	TimeReqSocket.recv(&Reply);
	std::string  TimeStampReply = std::string(static_cast<char*>(Reply.data()), Reply.size());
	LogReply(TimeStampReply);

	TimeReqSocket.close();
}

GazeStruct FPupilLabsUtils::GetGazeStructure()
{
	// Receive data message
	zmq::message_t topic;
	SubSocket->recv(&topic);
	zmq::message_t info;
	SubSocket->recv(&info);
	GazeStruct ReceivedGazeStruct = ConvertMsgPackToGazeStruct(std::move(info));
	return ReceivedGazeStruct;
}

void FPupilLabsUtils::InitializeCalibration()
{
	//Calibration parameters
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, TEXT("Initializing Calibration"));
	SamplesToIgnoreForEyeMovement = 40;
	CurrentCalibrationSamples = 0;
	bCalibrationStarted = true;
	bCalibrationEnded = false;
	CustomCalibration();
}

bool FPupilLabsUtils::CanGaze()
{
	// Boolean for opening up full data stream from PL after calibration
	// Used by MyTestPupilActor
	if (bCalibrationStarted && bCalibrationEnded)
		{
			can_gaze = true;
		}
	else
	{
		can_gaze = false;
	}
	return can_gaze;
}

void FPupilLabsUtils::SaveData(FString SaveText)
{
	// Save data to text file function
	FString text = *(FPaths::ProjectConfigDir() + UTF8TEXT("SaveFileTest"));
	FFileHelper::SaveStringToFile(SaveText, *(FPaths::ProjectConfigDir() + UTF8TEXT("SaveFileTest")), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}

void FPupilLabsUtils::CustomCalibration()
{
	// Set calibration locations; todo fix to make in front of HoloLens at start position
	CalibrationLocations.push_back(FVector(75, 0, 0));
	CalibrationLocations.push_back(FVector(75, 10, 10));
	CalibrationLocations.push_back(FVector(75, -10, 10));
	CalibrationLocations.push_back(FVector(75, 10, -10));
	CalibrationLocations.push_back(FVector(75, -10, -10));


	// Place initial calibration point
	calPoints = 0;
	IgnoreSamples = 0;
	CalibrationMarker->SetActorLocation(CalibrationLocations[calPoints]);
	bCalibrationProgressing = true;
}

Eigen::Matrix3f FPupilLabsUtils::Wahba(std::vector<Eigen::Vector3f> eyeLines, std::vector<Eigen::Vector3f> headLines)
{
	// Calculation for Wahba's problem to align rotation matrices
	Eigen::Matrix3f B;
	B.setZero();

	Eigen::Matrix3f R;
	R.setZero();

	for (int i = 0; i < size(eyeLines); i++)
	{
		B += eyeLines[i] * headLines[i].transpose();
		//UE_LOG(LogTemp, Warning, TEXT("eyeLines is %s"), *FVector(eyeLines[i](0), eyeLines[i](1), eyeLines[i](2)).ToString());
		//UE_LOG(LogTemp, Warning, TEXT("headLines is %s"), *FVector(headLines[i](0), headLines[i](1), headLines[i](2)).ToString());
	}

	Eigen::JacobiSVD<Eigen::Matrix3f> svd(B, Eigen::ComputeFullU | Eigen::ComputeFullV);
	auto U = svd.matrixU();
	auto V = svd.matrixV();
	float detU = U.determinant();
	float detV = V.determinant();

	Eigen::Vector3f vecM(1, 1, detU*detV);
	Eigen::Matrix3f M = vecM.asDiagonal();

	R = (U * M * V.transpose()).transpose(); // rotation matrix for reference to body frame
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, TEXT("Print R"));
	UE_LOG(LogTemp, Warning, TEXT("Row 1 is: %f, %f, %f"), R.coeff(0, 0), R.coeff(0, 1), R.coeff(0, 2)); // DO NOT DELETE (SAVE FOR REFERENCE)
	UE_LOG(LogTemp, Warning, TEXT("Row 2 is: %f, %f, %f"), R.coeff(1, 0), R.coeff(1, 1), R.coeff(1, 2));
	UE_LOG(LogTemp, Warning, TEXT("Row 3 is: %f, %f, %f"), R.coeff(2, 0), R.coeff(2, 1), R.coeff(2, 2));
	return R;
}

void FPupilLabsUtils::SetCalibrationMarker(ACalibrationMarker* MarkerRef, UWorld* World)
{
	// Assign calibration marker through MyTestPupilActor
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, TEXT("Initializing Calibration"));
	CalibrationMarker = MarkerRef;
	WorldRef = World;
	InitializeCalibration();
}

void FPupilLabsUtils::UpdateCustomCalibration()
{
	// Update custom calibration as data is collected
	// TODO fix for both eyes
	if (bCalibrationProgressing)
	{
		GazeStruct GazeData = GetGazeStructure();
		if (GazeData.confidence > 0.6 && GazeData.topic == "gaze.3d.01.")
		{
			//UE_LOG(LogTemp, Warning, TEXT("Confidence %f "), GazeData.confidence);
			//UE_LOG(LogTemp, Warning, TEXT("Topic %s "), *FString(GazeData.topic.c_str()));
			if (IgnoreSamples > SamplesToIgnoreForEyeMovement) // Ignore a few samples to account for people tracking the object
			{
				// Get headset position and orientation
                APlayerCameraManager* camManager = WorldRef->GetFirstPlayerController()->PlayerCameraManager;
                FVector HMDposition = camManager->GetCameraLocation();
				FQuat HMDorientation = camManager->GetCameraRotation().Quaternion();

				// Guess for eye positions and locations-- todo make math better
				FVector eye_offset_right_ue(-4, 3.15, -1.5);
				FVector eye_offset_left_ue(-4, -3.15, -1.5);
				Eigen::Vector3f e_r(-4, 3.15, -1.5);
				Eigen::Vector3f e_l(-4, -3.15, -1.5);
				eye_loc_right = e_r;
				eye_loc_left = e_l;
         
				// Calculate Calibration object relative transform to 
                FTransform HMDTransform = FTransform(HMDorientation, HMDposition+eye_offset_right_ue);
                static const FQuat Identity;
                FTransform CalTransform = FTransform(Identity, CalibrationLocations[calPoints]);
                FTransform CaltoHMD = UKismetMathLibrary::MakeRelativeTransform(CalTransform, HMDTransform);
				std::map<std::string, vector_3d> gaze_normals_3d = GazeData.gaze_normals_3d;
				for (std::map<std::string, vector_3d>::iterator it = gaze_normals_3d.begin(); it != gaze_normals_3d.end(); ++it)
				{
					std::string eye_d = it->first;
					vector_3d eye_vec = it->second;
					if (it->first == "0")
					{
						gazeDir_right.push_back(Eigen::Vector3f(it->second.x, it->second.y, it->second.z).normalized());
					}
					else if (it->first == "1")
					{
						gazeDir_left.push_back(Eigen::Vector3f(it->second.x, it->second.y, it->second.z).normalized());
					}
				}
                // eyeLoc_right.push_back(Eigen::Vector3f(GazeData.eye_center_3d.x * 10, GazeData.eye_center_3d.y * 10, GazeData.eye_center_3d.z * 10));
                calibrationLocationHeadsetFrame_right.push_back(Eigen::Vector3f(CaltoHMD.GetLocation()[0], CaltoHMD.GetLocation()[1], CaltoHMD.GetLocation()[2]));
                calibrationDirectionHeadsetFrame_right.push_back(Eigen::Vector3f(CaltoHMD.GetLocation()[0] - (HMDposition[0] + eye_loc_right[0]), (CaltoHMD.GetLocation()[1] - (HMDposition[1] + eye_loc_right[1])), CaltoHMD.GetLocation()[2] - (HMDposition[2] + eye_loc_right[2])).normalized());
				CurrentCalibrationSamples++;//Increment the current calibration sample. (Default sample amount per calibration point is 120)
			}
			IgnoreSamples++;//Increment the current calibration sample. (Default sample amount per calibration point is 120)
		}

		if (CurrentCalibrationSamples >= CurrentCalibrationSamplesPerDepth)
		{
			CurrentCalibrationSamples = 0;
			IgnoreSamples = 0;
			calPoints++;
			CalibrationMarker->SetActorLocation(CalibrationLocations[calPoints]);
			UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, TEXT("MoveCalActor"));
		}

		if (calPoints > 4)
		{
			bCalibrationProgressing = false;
			Rotation_r = Wahba(gazeDir_right, calibrationDirectionHeadsetFrame_right);
			Eigen::Quaternionf q(Rotation_r);
			FRotator UERot = FRotator(FQuat(q.x(), q.y(), q.z(), q.w()));
			UE_LOG(LogTemp, Warning, TEXT("Vector: %f %f %f"), UERot.Pitch, UERot.Roll, UERot.Yaw);
			bCalibrationEnded = true;
		}
	}
}

Eigen::Matrix3f FPupilLabsUtils::GetRotation_R()
{
	// For communicating data to Unreal Engine
	return Rotation_r;
}

Eigen::Matrix3f FPupilLabsUtils::GetRotation_L()
{
	// For communicating data to Unreal Engine
	return Rotation_l;
}

Eigen::Vector3f FPupilLabsUtils::GetLocation_R()
{
	// For communicating data to Unreal Engine
	return eye_loc_right;
}

Eigen::Vector3f FPupilLabsUtils::GetLocation_L()
{
	// For communicating data to Unreal Engine
	return eye_loc_left;
}