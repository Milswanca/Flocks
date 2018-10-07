// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FlocksAIController.generated.h"

/**
 * 
 */
UCLASS()
class FLOCKS4UE4_API AFlocksAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void Tick(float _deltaTime) override;
	float GetNeighbourDistanceSqr() const;

	virtual void Possess(APawn* InPawn) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetDirection(FVector _direction);
	FVector GetDirection() const;

	void MovedIntoRestriction(class AFlocksVolume* _volume);
	void MovedOutOfRestriction(class AFlocksVolume* _volume);
	bool IsInRestrictionVolume() const;

	int32 GetID() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float neighbourDist = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float rotationInterpSpeed = 5.0f;

private:
	FVector m_desiredDirection;
	FVector m_direction;

	UPROPERTY()
		int32 m_id;

	UPROPERTY()
		class AFlocksManager* m_flocksManager;

	UPROPERTY()
		TArray<class AFlocksVolume*> m_restrictionVolumes;
};
