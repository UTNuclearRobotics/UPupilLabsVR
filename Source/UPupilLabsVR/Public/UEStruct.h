#pragma once
#include "CoreMinimal.h"
#include "UEStruct.generated.h"

USTRUCT(BlueprintType)

struct FUEStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
	float id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
	FVector eye_loc;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pupil Data")
	FVector gaze_dir;
};