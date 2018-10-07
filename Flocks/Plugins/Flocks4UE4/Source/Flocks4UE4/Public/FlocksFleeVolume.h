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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FleeVolume")
		float InnerRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FleeVolume")
		float OuterRadius = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FleeVolume")
		float Falloff = 1.0f;

	float GetInfluence(FVector _position) const;

private:
	class AFlocksManager* m_flocksManager = nullptr;
};
