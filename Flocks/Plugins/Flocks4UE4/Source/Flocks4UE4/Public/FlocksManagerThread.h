// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "CoreMinimal.h"

struct FBoidData
{
public:
	int boidID = 0;
	FVector boidLocation;
	FVector boidVelocity;
	float boidRadius;

	FBoidData(int _boidID, FVector _boidLocation, FVector _boidVelocity, float _boidRadius)
	{
		boidID = _boidID;
		boidLocation = _boidLocation;
		boidVelocity = _boidVelocity;
		boidRadius = _boidRadius;
	}
};

struct FFleeVolumeData
{
public:

};

struct FBoidComputedData
{
public:
	FBoidComputedData(int _boidID, FVector _direction)
	{
		boidID = _boidID;
		direction = _direction;
	}
	
	int boidID = 0;
	FVector direction;
};

/**
 * 
 */
 //~~~~~ Multi Threading ~~~
class FFlocksManagerThread : public FRunnable
{
	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** The Data */
	TArray<FBoidData>* allBoids;
	TArray<FBoidComputedData>* computedData;
	TArray<int>* boidsToCompute;

	/** The PC */
	class AFlocksManager* FlockManager;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;

	FBoidComputedData UpdateBoid(FBoidData _boid);

	void CalculateNeighbours(FBoidData _boid, TArray<FBoidData>& _outNeighbours);
	FVector CalculateCohesion(FBoidData _boid, TArray<FBoidData>& _neighbours);
	FVector CalculateSeparation(FBoidData _boid, TArray<FBoidData>& _neighbours);
	FVector CalculateAlignment(FBoidData _boid, TArray<FBoidData>& _neighbours);
	FVector CalculateTowardsZero(FBoidData _boid);

	FCriticalSection m_mutex;

	int boidToCompute = 0;

public:

	//Done?
	bool IsFinished() const
	{
		return computedData->Num() == boidsToCompute->Num();
	}

	//~~~ Thread Core Functions ~~~

	//Constructor / Destructor
	FFlocksManagerThread(AFlocksManager* _flocksManager, TArray<FBoidData>* _allBoids, TArray<int>* _boidsToCompute, TArray<FBoidComputedData>* _computedData);
	virtual ~FFlocksManagerThread();

	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();
};