// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CalibrationMarker.generated.h"

UCLASS()
class UPUPILLABSVR_API ACalibrationMarker : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACalibrationMarker();
	UStaticMeshComponent* meshName = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MyMesh"));

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
