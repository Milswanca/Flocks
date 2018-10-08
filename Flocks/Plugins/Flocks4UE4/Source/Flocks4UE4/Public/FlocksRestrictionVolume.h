// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlocksVolume.h"
#include "FlocksRestrictionVolume.generated.h"

/**
 * 
 */
UCLASS()
class FLOCKS4UE4_API AFlocksRestrictionVolume : public AFlocksVolume
{
	GENERATED_BODY()
	
public:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	virtual EVolumeType GetVolumeType() const { return EVolumeType::VT_Restriction; }

protected:
	virtual void BeginPlay() override;
};
