#pragma once
#include "CoreMinimal.h"
#include "UEStruct.generated.h"

USTRUCT(BlueprintType)

struct FUEStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
		float confidence;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
		FString eye_tracker;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
		FVector eye_loc_r;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
		FVector gaze_dir_r;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
		FQuat gaze_rot_r;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
		float pupil_d_r;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
		FVector eye_loc_l;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
		FVector gaze_dir_l;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
		FQuat gaze_rot_l;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
		float pupil_d_l;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
		FVector gaze_point;
};