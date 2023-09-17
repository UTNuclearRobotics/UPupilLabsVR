// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Chifor Tudor

#include "FPupilLabsUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include <sstream>
#include <Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>

FPupilLabsUtils::FPupilLabsUtils()
{//Todo AndreiQ : De ce se cheama de doua ori ?
	UE_LOG(LogTemp, Warning, TEXT("FPupilLabsutil>>>>Initialized"));
	zmq::socket_t ReqSocket = ConnectToZmqPupilPublisher(Port);
	SubSocket = ConnectToSubport(&ReqSocket, PupilTopic);

	SynchronizePupilServiceTimestamp();
	
	// StartCalibration(&ReqSocket); // Run the calibration in Pupil Player, not here
	// InitializeCalibration(&ReqSocket);
	// FPlatformProcess::Sleep(10);
	// FPupilLabsUtils::CustomCalibration(); 
	
	// StopCalibration(&ReqSocket);
	// UpdateCalibration(&ReqSocket);

	//bCalibrationEnded = true;
	//bCalibrationStarted = true;
	//SetDetectionMode(&ReqSocket);
	//StartEyeProcesses(&ReqSocket);
	//Todo Close All Sockets within an ArrayList of Sockets

	//CALIBRATION
	//Implementeaza ceva logica cand sa pornest Update Calibration
	



   	ReqSocket.close();
}

FPupilLabsUtils::~FPupilLabsUtils()
{
	ZmqContext->close();
	if (!bSubSocketClosed)
	{// If the socket is already closed don't throw a npe
		SubSocket->close();
	}
	ZmqContext = nullptr;
	SubSocket = nullptr;
}

/**
* \Function which connects to the ZMQ Socket of the Pupul with a given Addr and Req_Port
* \param Addr Ip Adress or localhost.
* \param Reqport Port on which Pupil Capture is configured.
* \return ReqSocket the afformentioned ZMQ Socket
*/
zmq::socket_t FPupilLabsUtils::ConnectToZmqPupilPublisher(const std::string ReqPort) {
	ZmqContext = new zmq::context_t(1);
	std::string ConnAddr = Addr + ReqPort;
	zmq::socket_t ReqSocket(*ZmqContext, ZMQ_REQ);
	ReqSocket.connect(ConnAddr);

	return ReqSocket; //Todo Return Error Flag
}

/**
* \brief Takes the current Req Socket and request a subport on which it opens a connection to Pupil Service by binding a SubSocket 
* \param ReqSocket
*/
zmq::socket_t* FPupilLabsUtils::ConnectToSubport(zmq::socket_t *ReqSocket,const std::string Topic)
{
	/* Send Request for Sub Port */
	std::string SendSubPort = u8"SUB_PORT";
	zmq::message_t subport_request(SendSubPort.size());
	memcpy(subport_request.data(), SendSubPort.c_str(), SendSubPort.length());
	ReqSocket->send(subport_request);
	/* SUBSCRIBER SOCKET */
	std::string SubPortAddr = Addr + ReceiveSubPort(ReqSocket);
	zmq::socket_t* SubSocketTemp = new zmq::socket_t(*ZmqContext, ZMQ_SUB);
	SubSocketTemp->connect(SubPortAddr);
	SubSocketTemp->setsockopt(ZMQ_SUBSCRIBE, Topic.c_str(), Topic.length());
	bSubSocketClosed = false;

	return SubSocketTemp;
}

GazeStruct FPupilLabsUtils::ConvertMsgPackToGazeStruct(zmq::message_t info)
{
	char* payload = static_cast<char*>(info.data());
	msgpack::object_handle oh = msgpack::unpack(payload, info.size());
	msgpack::object deserialized = oh.get();
	GazeStruct ReceivedGazeStruct;

	std::stringstream ss;
	ss << deserialized;
	std::string demo = ss.str();
	deserialized.convert(ReceivedGazeStruct);
	FString SaveText = UTF8_TO_TCHAR(demo.c_str());
	SaveData(SaveText);
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

/**Todo Must be placed at the start of the Calibration*/
void FPupilLabsUtils::SynchronizePupilServiceTimestamp()
{
	//This is a different Socket such that it does not interfere with the other sockets //TODO Ask andrei if this is the best approuch
	zmq::socket_t TimeReqSocket = ConnectToZmqPupilPublisher(Port);
	
	float CurrentUETimestamp = FPlatformTime::Seconds();
	std::string SendCurrentUETimeStamp = u8"T " + std::to_string(CurrentUETimestamp);
	FString SendTimestamp(SendCurrentUETimeStamp.c_str());
	UE_LOG(LogTemp, Warning, TEXT("Current TimeStamp %s "), *SendTimestamp);

	zmq::message_t TimestampSendRequest(SendCurrentUETimeStamp.length());
	memcpy(TimestampSendRequest.data(), SendCurrentUETimeStamp.c_str(), SendCurrentUETimeStamp.length());
	TimeReqSocket.send(TimestampSendRequest);
	//We always have to receive the data so it is non blocking
	zmq::message_t Reply;
	TimeReqSocket.recv(&Reply);
	std::string  TimeStampReply = std::string(static_cast<char*>(Reply.data()), Reply.size());
	LogReply(TimeStampReply); //ToDo delete after implementation
	TimeReqSocket.close();

}

GazeStruct FPupilLabsUtils::GetGazeStructure()
{
	zmq::message_t topic;
	SubSocket->recv(&topic);
	zmq::message_t info;
	SubSocket->recv(&info);
	GazeStruct ReceivedGazeStruct = ConvertMsgPackToGazeStruct(std::move(info));
	return ReceivedGazeStruct;
}

void FPupilLabsUtils::InitializeCalibration()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, TEXT("Initializing Calibration"));
	SamplesToIgnoreForEyeMovement = 40;
	CurrentCalibrationPoint = 0;
	CurrentCalibrationSamples = 0;
	CurrentCalibrationDepth = 0;
	PreviousCalibrationDepth = -1;
	PreviousCalibrationPoint = -1;
	CalibrationElementIterator = 0;
	bCalibrationStarted = true;
	bCalibrationEnded = false;
	CustomCalibration();
	//CREATE FIRST MARKER
}

bool FPupilLabsUtils::CanGaze()
{
	if (bCalibrationStarted && bCalibrationEnded)
		{
			return true;
		}
	else return false;
}

void FPupilLabsUtils::SetCalibrationSceneVisualReference(AAPupilLabsVisualMarkersPawn* CalibrationScenePawn)
{
	VisualMarkersPawn = CalibrationScenePawn;
}

void FPupilLabsUtils::StartHMDPlugin(zmq::socket_t *ReqSocket)
{
	///DATA MARSHELLING
	StartPluginStruct StartPluginStruct = { u8"start_plugin" , PupilPluginName };
	std::string FirstBuffer = "notify." + StartPluginStruct.subject;

	zmq::message_t FirstFrame(FirstBuffer.size());
	memcpy(FirstFrame.data(), FirstBuffer.c_str(), FirstBuffer.size());

	msgpack::sbuffer SecondBuf;
	msgpack::pack(SecondBuf, StartPluginStruct);
	zmq::message_t SecondFrame(SecondBuf.size());
	memcpy(SecondFrame.data(), SecondBuf.data(), SecondBuf.size());
	//DATA SENDING
	zmq::multipart_t multipart;

	multipart.add(std::move(FirstFrame));
	multipart.add(std::move(SecondFrame));
	multipart.send(*ReqSocket);

	zmq::message_t Reply;
	ReqSocket->recv(&Reply);

	std::string  HMDPluginReply = std::string(static_cast<char*>(Reply.data()), Reply.size());
	LogReply(HMDPluginReply); //ToDo delete after implementation
}

void FPupilLabsUtils::SendCalibrationShouldStart(zmq::socket_t *ReqSocket)
{
	CalibrationShouldStartStruct ShouldStartStruct = { "calibration.should_start",{ 1200, 1200 }, 35,{ -15,0,0 },{ -15,0,0 } };
	std::string FirstBuffer = "notify." + ShouldStartStruct.subject;

	zmq::message_t FirstFrame(FirstBuffer.size());
	memcpy(FirstFrame.data(), FirstBuffer.c_str(), FirstBuffer.size());

	msgpack::sbuffer SecondBuf;
	msgpack::pack(SecondBuf, ShouldStartStruct);
	zmq::message_t SecondFrame(SecondBuf.size());
	memcpy(SecondFrame.data(), SecondBuf.data(), SecondBuf.size());
	//DATA SENDING
	zmq::multipart_t multipart;

	multipart.add(std::move(FirstFrame));
	multipart.add(std::move(SecondFrame));
	multipart.send(*ReqSocket);

	zmq::message_t Reply;
	ReqSocket->recv(&Reply);
}

void FPupilLabsUtils::StartCalibration(zmq::socket_t* ReqSocket)
{
	//INITIALIZE VISUAL DATA
	InitializeCalibration();
	StartHMDPlugin(ReqSocket);
	SendCalibrationShouldStart(ReqSocket);
	StartEyeProcesses(ReqSocket);
	//CloseEyeProcesses(ReqSocket);
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, TEXT("Calibration Started"));
	bCalibrationStarted = true;

}
void FPupilLabsUtils::StopCalibration(zmq::socket_t* ReqSocket)
{
	///DATA MARSHELLING
	CalibrationShouldStopStruct CalibrationShouldStopStruct = { "calibration.should_stop"};
	std::string FirstBuffer = "notify." + CalibrationShouldStopStruct.subject;

	zmq::message_t FirstFrame(FirstBuffer.size());
	memcpy(FirstFrame.data(), FirstBuffer.c_str(), FirstBuffer.size());
	msgpack::sbuffer SecondBuf;
	msgpack::pack(SecondBuf, CalibrationShouldStopStruct);
	zmq::message_t SecondFrame(SecondBuf.size());
	memcpy(SecondFrame.data(), SecondBuf.data(), SecondBuf.size());
	//DATA SENDING
	zmq::multipart_t multipart;
	multipart.add(std::move(FirstFrame));
	multipart.add(std::move(SecondFrame));
	multipart.send(*ReqSocket);
	zmq::message_t Reply;
	ReqSocket->recv(&Reply);
	
	bCalibrationEnded = true;

}

bool FPupilLabsUtils::SetDetectionMode(zmq::socket_t *ReqSocket)
{
	DetectionModeStruct DetectStruct = { u8"set_detection_mapping_mode" , u8"2d" };
	std::string FirstBuffer ="notify." + DetectStruct.subject;

	zmq::message_t FirstFrame(FirstBuffer.size());
	memcpy(FirstFrame.data(), FirstBuffer.c_str(), FirstBuffer.size());
	
	msgpack::sbuffer SecondBuf;
	msgpack::pack(SecondBuf, DetectStruct);
	zmq::message_t SecondFrame(SecondBuf.size());
	memcpy(SecondFrame.data(), SecondBuf.data(), SecondBuf.size());
	//DATA SENDING
	zmq::multipart_t multipart;

	multipart.add(std::move(FirstFrame));
	multipart.add(std::move(SecondFrame));

	multipart.send(*ReqSocket);

	zmq::message_t Reply;
	ReqSocket->recv(&Reply);

	std::string  Notification2DReply = std::string(static_cast<char*>(Reply.data()), Reply.size());
	LogReply(Notification2DReply); //ToDo delete after implementation
	return true;
}

void FPupilLabsUtils::StartEyeProcesses(zmq::socket_t *ReqSocket)
{
	bEyeProcess0 = StartEyeNotification(ReqSocket, "0");
	bEyeProcess1 = StartEyeNotification(ReqSocket, "1");
}

void FPupilLabsUtils::CloseEyeProcesses(zmq::socket_t *ReqSocket)
{
	bEyeProcess0 = CloseEyeNotification(ReqSocket, "0");
	bEyeProcess1 = CloseEyeNotification(ReqSocket, "1");
}

bool FPupilLabsUtils::StartEyeNotification(zmq::socket_t* ReqSocket, std::string EyeId)
{
	std::string Subject = "eye_process.should_start." + EyeId;
	
	float delay = 0; //Todo Refactorizat ciobanismul asta
	if (EyeId == "0") {
		delay = 0.1;
	}
	else
	{
		delay = 0.2;
	}
	
	EyeStruct EyeStruct = { Subject, atoi(EyeId.c_str()), delay};
	//zmq::socket_t* EyeSocket = new zmq::socket_t(*ZmqContext, ZMQ_PUB);
	zmq::socket_t EyeSocket = ConnectToZmqPupilPublisher(Port);
	std::string FirstBuffer = "notify." + Subject;
	msgpack::sbuffer SecondBuffer;
	msgpack::pack(SecondBuffer, EyeStruct);

	zmq::message_t FirstFrame(FirstBuffer.size());
	memcpy(FirstFrame.data(), FirstBuffer.c_str(), FirstBuffer.size());

	zmq::message_t SecondFrame(SecondBuffer.size());
	memcpy(SecondFrame.data(), SecondBuffer.data(), SecondBuffer.size());

	zmq::multipart_t multipart;
	multipart.add(std::move(FirstFrame));
	multipart.add(std::move(SecondFrame));

	multipart.send(*ReqSocket);

	zmq::message_t Reply;
	ReqSocket->recv(&Reply);

	std::string  Notification2DReply = std::string(static_cast<char*>(Reply.data()), Reply.size());
	LogReply(Notification2DReply);

	if (Notification2DReply == "Notification recevied.")
	{
		return true;
	}

	else
	{
		return false;
	}
}

bool  FPupilLabsUtils::CloseEyeNotification(zmq::socket_t* ReqSocket, std::string EyeId)
{
	std::string Subject = "eye_process.should_stop." + EyeId;

	EyeStruct EyeStruct = { Subject, atoi(EyeId.c_str()), 0.2 };
	//zmq::socket_t* EyeSocket = new zmq::socket_t(*ZmqContext, ZMQ_PUB);
	zmq::socket_t EyeSocket = ConnectToZmqPupilPublisher(Port);
	std::string FirstBuffer = "notify." + Subject;

	msgpack::sbuffer SecondBuffer;
	msgpack::pack(SecondBuffer, EyeStruct);

	zmq::message_t FirstFrame(FirstBuffer.size());
	memcpy(FirstFrame.data(), FirstBuffer.c_str(), FirstBuffer.size());

	zmq::message_t SecondFrame(SecondBuffer.size());
	memcpy(SecondFrame.data(), SecondBuffer.data(), SecondBuffer.size());

	zmq::multipart_t multipart;
	multipart.add(std::move(FirstFrame));
	multipart.add(std::move(SecondFrame));

	multipart.send(*ReqSocket);

	zmq::message_t Reply;
	ReqSocket->recv(&Reply);

	std::string  Notification2DReply = std::string(static_cast<char*>(Reply.data()), Reply.size());
	LogReply(Notification2DReply);

	if (Notification2DReply == "Notification recevied.")
	{
		return false;
	}
	else
	{
		return true;
	}
}

//private static List<Dictionary<string, object>> _calibrationData = new List<Dictionary<string, object>>();
///THIS SEND THE DATA 
void FPupilLabsUtils::AddCalibrationReferenceData()
{
	//Send(new Dictionary<string, object>{
	//	{ "subject","calibration.add_ref_data" },
	//		{
	//			"ref_data",
	//			_calibrationData.ToArray()
	//		}
	//	});
	CalibrationStruct = {}; //Clear Data

}
////ToDo Find the appropriat struct collect data with reference position (bellow) and send data with reference data (above)
///// THIS GETS THE DATA
//void FPupilLabsUtils::AddCalibrationPointReferencePosition(float CalibrationData[3], float timestamp)
//{
//
//	//get marker data
//
//	/*if (CalibrationMode == Calibration.Mode._3D)
//		for (int i = 0; i < position.Length; i++)
//			position[i] *= PupilSettings.PupilUnitScalingFactor;
//
//	_calibrationData.Add(new Dictionary<string, object>() {
//		{ CalibrationType.positionKey, position },
//		{ "timestamp", timestamp },
//		{ "id", int.Parse(PupilData.rightEyeID) }
//	});
//	_calibrationData.Add(new Dictionary<string, object>() {
//		{ CalibrationType.positionKey, position },
//		{ "timestamp", timestamp },
//		{ "id", int.Parse(PupilData.leftEyeID) }
//	});*/
//}
//

///THIS USES THE ADD CALIBRATION POINT REFERENCE
void FPupilLabsUtils::UpdateCalibration(zmq::socket_t* ReqSocket)
{
	LastTimestamp = 0;
	float t = FPlatformTime::Seconds();
	if(t- LastTimestamp>TimeBetweenCalibrationPoints)
	{
		LastTimestamp = t;

		UpdateCalibrationPoint();

		if (CurrentCalibrationSamples > SamplesToIgnoreForEyeMovement)
		{
			FPupilLabsUtils::AddCalibrationPointReferencePosition(t);
		}
	}
	CurrentCalibrationSamples++;//Increment the current calibration sample. (Default sample amount per calibration point is 120)

	if (CurrentCalibrationSamples >= CurrentCalibrationSamplesPerDepth)
	{
		CurrentCalibrationSamples = 0;
		CurrentCalibrationDepth++; //For The calibration point 3d

		if (CurrentCalibrationDepth >= CurrentCalibrationTypeVectorDepthRadiusLength)
		{
			CurrentCalibrationDepth = 0;
			CurrentCalibrationPoint++;

			//Send the current relevant calibration data for the current calibration point. _CalibrationPoints returns _calibrationData as an array of a Dictionary<string,object>.
			FPupilLabsUtils::AddCalibrationReferenceData();
		}

			if (CurrentCalibrationPoint >= CalibrationType2DPointsNumber)

			{
			FPupilLabsUtils::StopCalibration(ReqSocket);
			}

	}

}
//VISUAL METHOD
/////DRAW AND UPDATE POSITION OF CALIBRATION POINT
void FPupilLabsUtils::UpdateCalibrationPoint()
{
	Offset = 0;

	if (CurrentCalibrationPoint > 0 && CurrentCalibrationPoint < CalibrationType2DPointsNumber)
	{
		//GET VISUAL MARKERPAWN
		VisualMarkersPawn->UpdatePosition(CurrentCalibrationPoint);
		CurrentCalibrationPointPositionX += CalibrationRadius * (float)FMath::Cos(2 * PI * (float)(CurrentCalibrationPoint - 1) / (float)(CalibrationType2DPointsNumber - 1));
		CurrentCalibrationPointPositionY += CalibrationRadius * (float)FMath::Sin(2 * PI * (float)(CurrentCalibrationPoint - 1) / (float)(CalibrationType2DPointsNumber - 1)) - 10;
		//Pack as Float Array
		
	}

	//DRAW MARKER
	//Marker.UpdatePosition(currentCalibrationPointPosition);
	//Marker.SetScale(type.markerScale);
}

void FPupilLabsUtils::AddCalibrationPointReferencePosition(float timestamp)
{
	int Id0 = 0;
	AddCalibrationReferenceElementStruct LeftEyeElement;
	LeftEyeElement.norm_pos.x = CurrentCalibrationPointPositionX;
	LeftEyeElement.norm_pos.y = CurrentCalibrationPointPositionY;
	LeftEyeElement.timestamp = timestamp;//Todo here is tricky
	LeftEyeElement.id = Id0;


	CalibrationElementIterator++;

	CalibrationStruct.ref_data[CalibrationElementIterator] = LeftEyeElement;

	int Id1 = 1;

	AddCalibrationReferenceElementStruct RighEyeElement;
	RighEyeElement.norm_pos.x = CurrentCalibrationPointPositionX;
	RighEyeElement.norm_pos.y = CurrentCalibrationPointPositionY;
	RighEyeElement.timestamp = timestamp;//Todo here is tricky
	RighEyeElement.id = Id1;

	CalibrationElementIterator++;

	CalibrationStruct.ref_data[CalibrationElementIterator] = RighEyeElement;
}

////Todo: Fix msgpack::sbuffer SecondBuffer; no * operator problem (Poate merge ca sstrream in loc de msgpack sbuffer dar ii prea riscant acuma
////void FPupilLabsUtils::SendMultiPartMessage(zmq::socket_t* ReqSocket, std::string FirstBuffer, msgpack::sbuffer SecondBuffer)
////{
////	zmq::message_t FirstFrame(FirstBuffer.size());
////	memcpy(FirstFrame.data(), FirstBuffer.c_str(), FirstBuffer.size());
////
////	zmq::message_t SecondFrame(SecondBuffer.size());
////	memcpy(SecondFrame.data(), SecondBuffer.data(), SecondBuffer.size());
////
////	zmq::multipart_t multipart;
////	multipart.add(std::move(FirstFrame));
////	multipart.add(std::move(SecondFrame));
////
////	multipart.send(*ReqSocket);
////}

void FPupilLabsUtils::SaveData(FString SaveText)
{
	FString text = *(FPaths::ProjectConfigDir() + UTF8TEXT("SaveFileTest"));
	FFileHelper::SaveStringToFile(SaveText, *(FPaths::ProjectConfigDir() + UTF8TEXT("SaveFileTest")), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}

void FPupilLabsUtils::CustomCalibration()
{
	CalibrationLocations.push_back(FVector(75, 0, 0));
	CalibrationLocations.push_back(FVector(75, 10, 10));
	CalibrationLocations.push_back(FVector(75, -10, 10));
	CalibrationLocations.push_back(FVector(75, 10, -10));
	CalibrationLocations.push_back(FVector(75, -10, -10));


	// place initial calibration point
	calPoints = 0;
	IgnoreSamples = 0;
	CalibrationMarker->SetActorLocation(CalibrationLocations[calPoints]);
	bCalibrationProgressing = true;
}

Eigen::Matrix3f FPupilLabsUtils::Wahba(std::vector<Eigen::Vector3f> eyeLines, std::vector<Eigen::Vector3f> headLines)
{
	Eigen::Matrix3f B;
	B.setZero();

	Eigen::Matrix3f R;
	R.setZero();

	for (int i = 0; i < size(eyeLines); i++)
	{
		B += eyeLines[i] * headLines[i].transpose();
		// UE_LOG(LogTemp, Warning, TEXT("eyeLines is %s"), *FVector(eyeLines[i](0), eyeLines[i](1), eyeLines[i](2)).ToString());
		// UE_LOG(LogTemp, Warning, TEXT("headLines is %s"), *FVector(headLines[i](0), headLines[i](1), headLines[i](2)).ToString());
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
	UE_LOG(LogTemp, Warning, TEXT("Row 2 is: %f, %f, %f"), R.coeff(1, 0), R.coeff(1, 1), R.coeff(1, 2)); // DO NOT DELETE (SAVE FOR REFERENCE)
	UE_LOG(LogTemp, Warning, TEXT("Row 3 is: %f, %f, %f"), R.coeff(2, 0), R.coeff(2, 1), R.coeff(2, 2)); // DO NOT DELETE (SAVE FOR REFERENCE)
	return R;
}

Eigen::Vector3f FPupilLabsUtils::LeastSquares(std::vector<Eigen::Vector3f> lsaPoints, std::vector<Eigen::Vector3f> lsaLines, Eigen::Matrix3f Rotation_mat)
{
	// perform LSA
	Eigen::Matrix3f identityMat;
	identityMat.setIdentity();

	Eigen::Matrix3f R;
	R.setZero();

	Eigen::Vector3f q(0, 0, 0);
	Eigen::Vector3f normnorm(0, 0, 0);
	Eigen::Vector3f transformedLines(0, 0, 0);
	// UE_LOG(LogTemp, Warning, TEXT("%s %d %s"), q(0), q(1), q(2));

	for (int i = 0; i < size(lsaPoints); i++)
	{
		// lsaLines[i] = -1*lsaLines[i];
		transformedLines = Rotation_mat * lsaLines[i];
		normnorm = transformedLines.normalized();
		// lsaLines[i] = -1 * lsaLines[i];
		R += identityMat - normnorm * normnorm.transpose();
		q += (identityMat - normnorm * normnorm.transpose()) * lsaPoints[i];
		UE_LOG(LogTemp, Warning, TEXT("transformedLines is %s"), *FVector(normnorm(0), normnorm(1), normnorm(2)).ToString());
		UE_LOG(LogTemp, Warning, TEXT("CalPoints is %s"), *FVector(lsaPoints[i](0), lsaPoints[i](1), lsaPoints[i](2)).ToString());
	}

	Eigen::Matrix3f R_pseudo = (R.transpose() * R).inverse() * R.transpose();
	Eigen::Vector3f solution_point = R_pseudo * q;
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, TEXT("Print Sol"));

	UE_LOG(LogTemp, Warning, TEXT("Result is %s"), *FVector(solution_point(0),solution_point(1),solution_point(2)).ToString()); // DO NOT DELETE (SAVE FOR REFERENCE)
	return solution_point;
}


void FPupilLabsUtils::TransformCalc(Eigen::Vector3f solution_point, std::vector<Eigen::Vector3f> headsetCalibrationPoints, std::vector<Eigen::Vector3f> gazeLines, std::vector<Eigen::Vector3f> eyePoints)
{
	std::vector<Eigen::Vector3f> gazeCalibrationPoints;
	for (int i = 0; i < size(eyePoints); i++)
	{
		float distance = sqrt(powf(solution_point[0] - headsetCalibrationPoints[i][0], 2) + powf(solution_point[1] - headsetCalibrationPoints[i][1], 2) + powf(solution_point[2] - headsetCalibrationPoints[i][2], 2));
		Eigen::Vector3f gazeCalibrationLoc = eyePoints[i] + distance * gazeLines[i];
		gazeCalibrationPoints.push_back(gazeCalibrationLoc);
	}

	Eigen::MatrixXf cameraPoints(4, size(eyePoints));
	Eigen::MatrixXf headsetPoints(4, size(eyePoints));

	for (int i = 0; i < size(eyePoints); i++)
	{
		cameraPoints.col(i) = gazeCalibrationPoints[i];
		cameraPoints(i,3) = 1;
		headsetPoints.col(i) = headsetCalibrationPoints[i];
		headsetPoints(i, 3) = 1;
	}

	Eigen::MatrixXf cameraPseudo(size(eyePoints), size(eyePoints));
	cameraPseudo = cameraPoints.transpose() * (cameraPoints * cameraPoints.transpose()).inverse();

	transform = headsetPoints * cameraPseudo;
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, TEXT("Print transform"));
	UE_LOG(LogTemp, Warning, TEXT("Row 1 is: %f, %f, %f, %f"), transform.coeff(0, 0), transform.coeff(0, 1), transform.coeff(0, 2), transform.coeff(0, 3)); // DO NOT DELETE (SAVE FOR REFERENCE)
	UE_LOG(LogTemp, Warning, TEXT("Row 2 is: %f, %f, %f, %f"), transform.coeff(1, 0), transform.coeff(1, 1), transform.coeff(1, 2), transform.coeff(1, 3)); // DO NOT DELETE (SAVE FOR REFERENCE)
	UE_LOG(LogTemp, Warning, TEXT("Row 3 is: %f, %f, %f, %f"), transform.coeff(2, 0), transform.coeff(2, 1), transform.coeff(2, 2), transform.coeff(2, 3)); // DO NOT DELETE (SAVE FOR REFERENCE)
	UE_LOG(LogTemp, Warning, TEXT("Row 4 is: %f, %f, %f, %f"), transform.coeff(3, 0), transform.coeff(3, 1), transform.coeff(3, 2), transform.coeff(3, 3)); // DO NOT DELETE (SAVE FOR REFERENCE)
}

void FPupilLabsUtils::SetCalibrationMarker(ACalibrationMarker* MarkerRef, UWorld* World)
{
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] : %s"), TEXT(__FUNCTION__), __LINE__, TEXT("Initializing Calibration"));
	CalibrationMarker = MarkerRef;
	WorldRef = World;
	// CalibrationMarker->SetActorLocation(FVector(0, 100, 0));
	InitializeCalibration();
}

void FPupilLabsUtils::UpdateCustomCalibration()
{
	if (bCalibrationProgressing)
	{
		GazeStruct GazeData = GetGazeStructure();
		if (GazeData.confidence > 0.6 && GazeData.topic == "gaze.3d.01.") // for one eye currently //&& GazeData.confidence>0.6 && !isnan(GazeData.gaze_normal_3d.x)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Confidence %f "), GazeData.confidence);
			//UE_LOG(LogTemp, Warning, TEXT("Topic %s "), *FString(GazeData.topic.c_str()));
			if (IgnoreSamples > SamplesToIgnoreForEyeMovement)
			{
                APlayerCameraManager* camManager = WorldRef->GetFirstPlayerController()->PlayerCameraManager;
                FVector HMDposition = camManager->GetCameraLocation();
                FQuat HMDorientation = camManager->GetCameraRotation().Quaternion();
                FTransform HMDTransform = FTransform(HMDorientation, HMDposition);
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
                calibrationDirectionHeadsetFrame_right.push_back(Eigen::Vector3f(CaltoHMD.GetLocation()[0] - HMDposition[0], CaltoHMD.GetLocation()[1] - HMDposition[1], CaltoHMD.GetLocation()[2] - HMDposition[2]).normalized());
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
			Rotation = Wahba(gazeDir_right, calibrationDirectionHeadsetFrame_right);
			Rotation_l = Wahba(gazeDir_left, calibrationDirectionHeadsetFrame_right);
			eye_loc_right = LeastSquares(calibrationLocationHeadsetFrame_right, gazeDir_right, Rotation);
			eye_loc_left = LeastSquares(calibrationLocationHeadsetFrame_right, gazeDir_left, Rotation_l);
			// Eigen::Vector3f eye_loc_right = LeastSquares(calibrationLocationHeadsetFrame_right, gazeDir_right);
			// Eigen::Vector3f eye_loc_left = LeastSquares(calibrationLocationHeadsetFrame_left, gazeDir_left);
			// calibrationLocationHeadsetFrame_right.insert(calibrationLocationHeadsetFrame_right.end(), calibrationLocationHeadsetFrame_left.begin(), calibrationLocationHeadsetFrame_left.end());
			// TransformCalc(eye_loc_right, calibrationLocationHeadsetFrame_right, gazeDir_right, eyeLoc_right);
			// TransformCalc(eye_loc_left, calibrationLocationHeadsetFrame_left, gazeDir_left, eyeLoc_left);

			bCalibrationEnded = true;
		}
	}
}

Eigen::Matrix3f FPupilLabsUtils::GetRotation()
{
	return Rotation;
}

Eigen::Vector3f FPupilLabsUtils::GetLocation()
{
	return eye_loc_right;
}