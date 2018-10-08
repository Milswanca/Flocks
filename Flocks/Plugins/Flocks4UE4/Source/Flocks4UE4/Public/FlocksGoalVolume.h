// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlocksVolume.h"
#include "FlocksGoalVolume.generated.h"

/**
 * 
 */
UCLASS()
class FLOCKS4UE4_API AFlocksGoalVolume : public AFlocksVolume
{
	GENERATED_BODY()
	
public:
	virtual EVolumeType GetVolumeType() const { return EVolumeType::VT_Goal; }
	
	
};
