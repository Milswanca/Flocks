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

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
		class AFlocksManager* m_flocksManager = nullptr;

	int32 m_id = -1;
};
