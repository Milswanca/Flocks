// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlocksVolume.h"
#include "FlocksFleeVolume.generated.h"

/**
 * 
 */
UCLASS()
class FLOCKS4UE4_API AFlocksFleeVolume : public AFlocksVolume
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual EVolumeType GetVolumeType() const { return EVolumeType::VT_Flee; }


};
