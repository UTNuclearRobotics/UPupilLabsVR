// Fill out your copyright notice in the Description page of Project Settings.


#include "CalibrationMarker.h"

// Sets default values
ACalibrationMarker::ACalibrationMarker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("/Game/StarterContent/Shapes/shape_sphere.shape_sphere"));

	if (meshAsset.Succeeded())
	{
		UE_LOG(LogTemp, Warning, TEXT("Success"));
		meshName->SetStaticMesh(meshAsset.Object);
		meshName->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		meshName->SetWorldScale3D(FVector(0.2f));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NoSuccess"));
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> FoundMaterial(TEXT("/Game/StarterContent/Materials/M_Basic_Wall_Inst.M_Basic_Wall_Inst"));
	if (FoundMaterial.Succeeded())
	{
		UMaterialInstanceDynamic* DynamicMaterialInst = UMaterialInstanceDynamic::Create(FoundMaterial.Object, meshName);
		meshName->SetMaterial(0, DynamicMaterialInst);
	}
}

// Called when the game starts or when spawned
void ACalibrationMarker::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACalibrationMarker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

