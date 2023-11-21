// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EyeTrackerFunctionLibrary.h"
#include "EyeTracker.generated.h"



UCLASS()
class UPUPILLABSVR_API AEyeTracker : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEyeTracker();
	UWorld* World;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};