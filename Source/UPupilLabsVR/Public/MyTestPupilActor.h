// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "FPupilMsgWorker.h"
#include "UEStruct.h"
#include "GazeStruct.h"
#include "CalibrationMarker.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MyTestPupilActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPupilDelegate, FUEStruct, GazeData);

UCLASS()
class UPUPILLABSVR_API AMyTestPupilActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMyTestPupilActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	///--BEGIN RAYCAST--///
	//Method for receiving the GazeStructure for The RayCasting
	void OnNewPupilData(GazeStruct* GazeStructure);
	//Received Data From The Worker's Event
	GazeStruct* ReceivedGazeStructure;
	///--END RAYCAST--///
	UWorld* World;
	Eigen::Vector3f Location_r;
	Eigen::Matrix3f Rotation_r;
	Eigen::Vector3f Location_l;
	Eigen::Matrix3f Rotation_l;
	Eigen::Quaternionf q_r;
	Eigen::Quaternionf q_l;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	std::unique_ptr<FPupilMsgWorker> PupilComm;
	UFUNCTION(BlueprintCallable, Category = "Pupil Labs", meta = (Keywords = "Access Gaze Ray"))
		FUEStruct PupilData();
	UFUNCTION(BlueprintCallable, Category = "Pupil Labs", meta = (Keywords = "Can Gaze"))
		bool CanGaze();
	UFUNCTION(BlueprintCallable, Category = "Pupil Labs", meta = (Keywords = "Start Calibration"))
		void StartCalibration();
	//DummyEvent& OnNewData() { return NewPupilDataEvent; }
	//DummyEvent NewPupilDataEvent;
	UPROPERTY(BlueprintAssignable)
		FPupilDelegate PupilDelegate;
	//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExampleDelegate_OnSomething, GazeStruct*, _exampleEventParameter);
	void SendData();
};