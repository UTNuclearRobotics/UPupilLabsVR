// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Chifor Tudor
// Updated by: Christina Petlowany, The University of Texas at Austin

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
	// Constructor. Initializes the Zmq context and req socket
	FPupilLabsUtils();
	// Destructor
	~FPupilLabsUtils();
	// Public Method accesible by the Worker Thread to get Gaze Structure Data
	GazeStruct GetGazeStructure();

	// Public Method To be called when we do not need to receive any data from pupil service
	void CloseSubSocket();

	void InitializeCalibration();

	bool CanGaze();
	bool can_gaze = false;


	ACalibrationMarker* CalibrationMarker;
	void SetCalibrationMarker(ACalibrationMarker* MarkerRef, UWorld* World);

	void UpdateCustomCalibration();

	Eigen::Matrix3f Rotation_r;
	Eigen::Matrix3f Rotation_l;
	Eigen::Vector3f eye_loc_right;
	Eigen::Vector3f eye_loc_left;
	Eigen::Matrix3f GetRotation_R();
	Eigen::Matrix3f GetRotation_L();
	Eigen::Vector3f GetLocation_R();
	Eigen::Vector3f GetLocation_L();

	FString GetWriteData();
	FString data_to_write;


private:
	bool WriteStringToProjectConfigFile(const FString Section, const FString FileName, const FString Key, const FString Value);
	bool ReadStringFromProjectConfigFile(const FString Section, const FString FileName, const FString Key, FString& Value);
	Eigen::Matrix3f GetEigenFromStringM(FString InputString);
	Eigen::Vector3f GetEigenFromStringV(FString InputString);
	FString GetStringFromEigen(Eigen::Matrix3f InputMatrix);
	FString GetStringFromEigen(Eigen::Vector3f InputVector);
	zmq::socket_t ConnectToZmqPupilPublisher(std::string ReqPort); // Connect to the Response Socket of the Pupil Service using a Request Socket
	zmq::socket_t* ConnectToSubport(zmq::socket_t* ReqSocket, const std::string Topic); // Connect to the Subport of the Publisher given from the Request Socket
	std::string ReceiveSubPort(zmq::socket_t *ReqSocket); // Receive and log the SubPort
	void SynchronizePupilServiceTimestamp(); // Synchronize the clock of Pupil Service with the current timestamp of this client. Pupil Service acts as a follower for this
	void LogReply(std::string SubportReply); // Logs the SubPort
	GazeStruct ConvertMsgPackToGazeStruct(zmq::message_t info); // Converts the binary message into a C structure using msgpack
	void SaveData(FString SaveText); // Saves the data to file
	void CustomCalibration(); // Manages the custom calibration
	Eigen::Matrix3f Wahba(std::vector<Eigen::Vector3f> eyeLines, std::vector<Eigen::Vector3f> headLines); // Performs Wahba's problem calculation

private:
	int CurrentCalibrationSamples;
	int SamplesToIgnoreForEyeMovement;
	const int CurrentCalibrationSamplesPerDepth = 240;
	zmq::context_t* ZmqContext;
	zmq::socket_t *SubSocket;
	bool bSubSocketClosed;
	std::vector<Eigen::Vector3f> calibrationLocationHeadsetFrame_right;
	std::vector<Eigen::Vector3f> calibrationDirectionHeadsetFrame_right;
	std::vector<Eigen::Vector3f> calibrationLocationHeadsetFrame_left;
	std::vector<Eigen::Vector3f> calibrationDirectionHeadsetFrame_left;
	std::vector<Eigen::Vector3f> gazeDir_right;
	std::vector<Eigen::Vector3f> eyeLoc_right;
	std::vector<Eigen::Vector3f> gazeDir_left;
	std::vector<Eigen::Vector3f> eyeLoc_left;
	int calPoints;
	int IgnoreSamples;
	bool bCalibrationProgressing = false;
	std::vector<FVector> CalibrationLocations;
	UWorld* WorldRef;

	// Parameters for connecting to PL
	std::string Addr = "tcp://127.0.0.1:";
	std::string Port = "50020";
	std::string PupilTopic = u8"gaze";
	std::string PupilPluginName = "HMD_Calibration"; // maybe not used todo
};

