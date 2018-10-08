// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlocksManagerThread.h"
#include "GameFramework/Actor.h"
#include "FlocksComputeShaderWrapper.h"
#include "FlocksManager.generated.h"

UENUM(BlueprintType)
enum class ComputeType : uint8
{
	CT_CPU,
	CT_MultiThreaded,
	CT_GPU
};

struct FThreadData
{
public:
	FThreadData() {}

	class FFlocksManagerThread* Thread = nullptr;
	TArray<FBoidComputedData> ComputedData;
	TArray<int> BoidsToCompute;
};

/**
 *
 */
UCLASS()
class FLOCKS4UE4_API AFlocksManager : public AActor
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual int32 RegisterBoid(class AFlocksAIController* _controller);
	virtual void DeregisterBoid(class AFlocksAIController* _controller);

	virtual int32 RegisterVolume(class AFlocksVolume* _volume);
	virtual void DeregisterVolume(class AFlocksVolume* _volume);

	virtual void UpdateBoids(float _deltaTime);

	void GetBoids(TArray<class AFlocksAIController*>& _outBoids);

	void BoidChangedAcceleration(int32 _id, float _acceleration);
	void BoidChangedMaxVelocity(int32 _id, float _maxVelocity);

	void CalculateNeighbours(AFlocksAIController* _controller, TArray<AFlocksAIController*>& _outNeighbours);
	FVector CalculateCohesion(class AFlocksAIController* _controller, TArray<class AFlocksAIController*> _neighbours);
	FVector CalculateSeparation(class AFlocksAIController* _controller, TArray<class AFlocksAIController*> _neighbours);
	FVector CalculateAlignment(class AFlocksAIController* _controller, TArray<class AFlocksAIController*> _neighbours);
	FVector CalculateTowardsZero(class AFlocksAIController* _controller);
	FVector CalculateFlee(class AFlocksAIController* _controller);
	FVector CalculateRestriction(class AFlocksAIController* _controller);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		ComputeType computeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float cohesionRadius = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float separationRadius = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float alignmentRadius = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float maxAcceleration = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float maxVelocity = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		FVector zeroLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float cohesionWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float separationWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float alignmentWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float toZeroWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float fleeWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		float restrictionWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		int32 numThreads = 4;

	//Killing the thread for example in EndPlay() or BeginDestroy()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;

private:
	bool initted = false;
	int initCount = 0;

	int32 m_currentThread = 0;

	TSharedPtr<class FlocksComputeShaderWrapper> m_flockShader;

	TArray<BoidState> m_boidData;
	TArray<FlocksVolumeData> m_volumeData;
	TArray<FThreadData> m_threadData;

	UPROPERTY()
	TArray<class AFlocksAIController*> m_boids;

	UPROPERTY()
		TArray<class AFlocksVolume*> m_flockVolumes;
};
