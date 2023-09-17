// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Chifor Tudor

#pragma once

#include "Core.h"
#include "Logging/LogMacros.h"
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "Engine/Engine.h"
#include <Eigen/Dense>
#include <vector>
#include <cmath>
#include "IXRTrackingSystem.h"
/**
* \MSGPACK_USE_CPP03 has been applied to use the CPP03 _t implementations instead of _type.
*  Any usage of CPP011 calls from msgpack will not work.
*  As of now the mvsc compiler does not work with CPP11.
*/
// #define   MSGPACK_USE_CPP03

#undef check
#include "msgpack.hpp"
#include "GazeStruct.h"
#include "APupilLabsVisualMarkersPawn.h"
#include "CalibrationMarker.h"

// 
/**
 * \brief A class to wrap all the communication done with the help of ZeroMQ and Msgpack 
 *	ZeroMq is a networking protocol whilst msgpack an encoding format used by the client from Pupil Service for maximum communication speed. You can think of msgpack as a JSON format but send in binary Instead of Human Readeble Format
 *	Also Other Strucs and values can be accessed from the this Helper class like :
 *	gaze
 *	notify
 *	pupil
 *	logging
 *	By changing the Topic filter (Upcomming feature)
 */
class FPupilLabsUtils
{
public:
	/** Constructor. Initializes the Zmq Context and Req Sockect*/
	FPupilLabsUtils();
	/** Destructor */
	~FPupilLabsUtils();
	/**Public Method accesible by the Worker Thread such that we can get the Gaze Structure Data*/
	GazeStruct GetGazeStructure();
	Eigen::Matrix4f transform;
	/**Public Method To be called when we do not need to receive any data from pupil service*/
	void CloseSubSocket();
	///CALIBRATION METHODS///
	/*
	 * Todo s: Calibration class 
	 * pass data thorugh hre
	 * pupiltools.cs
	 * Implement Multipart message
	 */
	/**Position float should be a vector todo*/
	void StartHMDPlugin(zmq::socket_t* ReqSocket);
	void SendCalibrationShouldStart(zmq::socket_t * ReqSocket);
	//CALIBRATION CLASS ?
	void StartCalibration(zmq::socket_t* socket);
	void StopCalibration(zmq::socket_t* socket);

	void InitializeCalibration();

	bool CanGaze();
	void SetCalibrationSceneVisualReference(AAPupilLabsVisualMarkersPawn* CalibrationScenePawn);



	void AddCalibrationReferenceData();
	void AddCalibrationPointReferencePosition(float timestamp);

	void UpdateCalibrationPoint();
	void UpdateCalibration(zmq::socket_t* ReqSocket);
	//CALIBRATION CLASS ?
	void StartEyeProcesses(zmq::socket_t* ReqSocket);
	bool StartEyeNotification(zmq::socket_t* ReqSocket, std::string EyeId);
	bool CloseEyeNotification(zmq::socket_t* ReqSocket, std::string EyeId);
	void SendMultiPartMessage(zmq::socket_t* ReqSocket, std::string FirstBuffer, msgpack::sbuffer SecondBuffer);
	bool StartEyeProcesses();
	void CloseEyeProcesses(zmq::socket_t* ReqSocket);

	ACalibrationMarker* CalibrationMarker;
	void SetCalibrationMarker(ACalibrationMarker* MarkerRef, UWorld* World);

	///END CALIBRATION METHODS///
	void UpdateCustomCalibration();

	Eigen::Matrix3f Rotation;
	Eigen::Matrix3f Rotation_l;
	Eigen::Vector3f eye_loc_right;
	Eigen::Vector3f eye_loc_left;
	Eigen::Matrix3f GetRotation();
	Eigen::Vector3f GetLocation();


private:
	///CALIBRATION METHODS
	bool SetDetectionMode(zmq::socket_t* ReqSocket);
	/**Method Used to send a multipart message with 2 Frames in Pupil Notification Style.First frame is a notify.request std::string and the second one is MsgPack Struct*/
//	void SendMultipartMessage(zmq::socket_t *SubSocket, zmq::message_t Reply);
	/**Method that starts connecting to the Response Socket of the Pupil Service using a Request Socket */
	zmq::socket_t ConnectToZmqPupilPublisher(std::string ReqPort);
	/**Method that connects to the Subport of the Publisher given from the Request Socket*/
	zmq::socket_t* ConnectToSubport(zmq::socket_t* ReqSocket, const std::string Topic);
	/**Method that receives and logs the SubPort */
	std::string ReceiveSubPort(zmq::socket_t *ReqSocket);
	/**Method  that synchronizes the clock of Pupil Service with the current timestamp of this client. Pupil Service acts as a follower for this  */
	void SynchronizePupilServiceTimestamp();
	 /**Helper Method that logs the SubPort */
	 void LogReply(std::string SubportReply);
	/**Helper Method that converts the binary message into a C structure using msgpack*/
	 GazeStruct ConvertMsgPackToGazeStruct(zmq::message_t info);
	 void SaveData(FString SaveText);
	 void CustomCalibration();
	 Eigen::Matrix3f Wahba(std::vector<Eigen::Vector3f> eyeLines, std::vector<Eigen::Vector3f> headLines);
	 Eigen::Vector3f LeastSquares(std::vector<Eigen::Vector3f> lsaPoints, std::vector<Eigen::Vector3f> lsaLines, Eigen::Matrix3f Rotation_Mat);
	 void TransformCalc(Eigen::Vector3f solution_point, std::vector<Eigen::Vector3f> headsetCalibrationPoints, std::vector<Eigen::Vector3f> gazeLines, std::vector<Eigen::Vector3f> eyePoints);
	
private:
	int CalibrationElementIterator;
	AddCalibrationReferenceDataStruct CalibrationStruct;

	AAPupilLabsVisualMarkersPawn * VisualMarkersPawn;
	bool bCalibrationStarted;
	bool bCalibrationEnded;
	///CALIBRATION PROCESS Todo Maybe a Settings struct
	 bool bEyeProcess0;
	 bool bEyeProcess1;
	 float LastTimestamp ;
	 int CurrentCalibrationSamples;
	 int SamplesToIgnoreForEyeMovement;
	 float Offset;
	 float Radius;
	 const float TimeBetweenCalibrationPoints = 0.02f;
	 const int CalibrationType2DPointsNumber = 8;
	 const int CurrentCalibrationSamplesPerDepth = 240;
	 const int CurrentCalibrationTypeVectorDepthRadiusLength = 2;
	 int CurrentCalibrationPoint;
	 int CurrentCalibrationDepth;
	 int PreviousCalibrationDepth;
	 int PreviousCalibrationPoint;
	 float CurrentCalibrationPointPosition[3];
	 const float VectorDepthRadius[2] = { 2.0, 0.07 };
	 float CalibrationRadius = 0.12;
	 float CurrentCalibrationPointPositionX, CurrentCalibrationPointPositionY;
	//END CALIBRATION PROCESS
	 /**Context for Zmq to rely on*/
	zmq::context_t* ZmqContext;
	/**Subsocket on which the Zmq Request Socket Subscribes*/
	zmq::socket_t *SubSocket;
	bool bSubSocketClosed;
	std::vector<Eigen::Vector3f> calibrationLocationHeadsetFrame_right;
	std::vector<Eigen::Vector3f> calibrationDirectionHeadsetFrame_right;
	std::vector<Eigen::Vector3f> calibrationLocationHeadsetFrame_left;
	std::vector<Eigen::Vector3f> gazeDir_right;
	std::vector<Eigen::Vector3f> eyeLoc_right;
	std::vector<Eigen::Vector3f> gazeDir_left;
	std::vector<Eigen::Vector3f> eyeLoc_left;
	int calPoints;
	int IgnoreSamples;
	bool bCalibrationProgressing;
	std::vector<FVector> CalibrationLocations;
	UWorld* WorldRef;

	/**Begin Hardcoded strings that define the connection and type of Subscription */
	std::string Addr = "tcp://127.0.0.1:";//TODO UPROPERTY Options
	std::string Port = "50020";
	std::string PupilTopic = u8"gaze"; //TODO Implement Options With UObject
	std::string PupilPluginName = "HMD_Calibration";

	/**End Hardcoded strings that define the connection and type of Subscription */
};

