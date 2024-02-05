// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Chifor Tudor
// Updated by: Christina Petlowany, The University of Texas at Austin

#pragma once

#include "Core/Public/HAL/Runnable.h"
#include "Core/Public/HAL/RunnableThread.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Core/Public/HAL/PlatformProcess.h"

#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
// #include "Editor.h"
#include "GameFramework/PlayerController.h"
#include "FPupilLabsUtils.h"
#include "CalibrationMarker.h"

/**
 * Thread which connects and receive data from a certain @PupilTopic from Pupil Serice.
 * Start- and StopListening are all what is needed to use this Thread.
 */
class FPupilMsgWorker : public FRunnable
{
public:
	void SetCalibrationMarker(ACalibrationMarker* MarkerRef, UWorld* World);
	void UpdateCalibration();
	Eigen::Matrix3f Rotation_r;
	Eigen::Matrix3f Rotation_l;
	Eigen::Vector3f Location_r;
	Eigen::Vector3f Location_l;
	Eigen::Matrix3f GetRotation_R();
	Eigen::Matrix3f GetRotation_L();
	Eigen::Vector3f GetLocation_R();
	Eigen::Vector3f GetLocation_L();
	bool can_gaze = true;
	bool CanGaze();
public:
	DECLARE_EVENT_OneParam(FPupilMsgWorker, DummyEvent, GazeStruct*);
	DummyEvent& OnNewData()  { return NewPupilDataEvent; }
	/** Constructor */
	FPupilMsgWorker();
	/** Destructor */
	virtual ~FPupilMsgWorker() override;

private:
	/* Begin FRunnable interface */
	bool Init() override;
	virtual void Stop() override;
	virtual uint32 Run() override; 
	/* End FRunnable interface */
	/** Makes sure this thread has stopped properly */

private:
	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	FPupilLabsUtils PupilHelper; //this is already an instance of the object
	//before the construtor code is called - ie at allocationn

	bool bSuccessfulyInit;

	bool bRunning;

	DummyEvent NewPupilDataEvent;

	GazeStruct ReceivedGazeStructure;
};
