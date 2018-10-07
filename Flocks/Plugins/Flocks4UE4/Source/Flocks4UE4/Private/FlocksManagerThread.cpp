#include "FlocksManagerThread.h"
#include "FlocksAIController.h"
#include "FlocksManager.h"

FFlocksManagerThread::FFlocksManagerThread(AFlocksManager* _flocksManager, TArray<FBoidData>* _allBoids, TArray<int>* _boidsToCompute, TArray<FBoidComputedData>* _computedData)
	: FlockManager(_flocksManager)
	, StopTaskCounter(0)
{
	//Link to where data should be stored
	allBoids = _allBoids;
	computedData = _computedData;
	boidsToCompute = _boidsToCompute;

	Thread = FRunnableThread::Create(this, TEXT("FFlocksManagerThread"), 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
}

FFlocksManagerThread::~FFlocksManagerThread()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool FFlocksManagerThread::Init()
{
	return true;
}

//Run
uint32 FFlocksManagerThread::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(5);

	//While not told to stop this thread 
	//		and not yet finished finding Prime Numbers
	while (StopTaskCounter.GetValue() == 0)
	{
		if (IsFinished()) { continue; }

		if (boidToCompute > boidsToCompute->Num() - 1)
		{
			boidToCompute = 0;
		}

		FBoidComputedData computed = UpdateBoid((*allBoids)[(*boidsToCompute)[boidToCompute]]);
		computedData->Add(computed);

		boidToCompute++;
	}

	//Run FFlocksManagerThread::Shutdown() from the timer in Game Thread that is watching
	//to see when FFlocksManagerThread::IsThreadFinished()

	return 0;
}

//stop
void FFlocksManagerThread::Stop()
{
	StopTaskCounter.Increment();
}

void FFlocksManagerThread::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

FBoidComputedData FFlocksManagerThread::UpdateBoid(FBoidData _boid)
{
	TArray<FBoidData> neighbours;
	CalculateNeighbours(_boid, neighbours);

	FVector finalDir = _boid.boidVelocity + CalculateCohesion(_boid, neighbours) + CalculateAlignment(_boid, neighbours) + CalculateSeparation(_boid, neighbours) + CalculateTowardsZero(_boid);
	finalDir.Normalize();

	return FBoidComputedData(_boid.boidID, finalDir);
}

void FFlocksManagerThread::CalculateNeighbours(FBoidData _boid, TArray<FBoidData>& _outNeighbours)
{
	float nDist = _boid.boidRadius;

	for (FBoidData i : (*allBoids))
	{
		if (i.boidID == _boid.boidID) { continue; }

		float dist = FVector::DistSquared(i.boidLocation, _boid.boidLocation);

		if (dist < nDist)
		{
			_outNeighbours.Add(i);
		}
	}
}

FVector FFlocksManagerThread::CalculateCohesion(FBoidData _boid, TArray<FBoidData>& _neighbours)
{
	if (_neighbours.Num() == 0) { return FVector::ZeroVector; }

	FVector cohesion = FVector::ZeroVector;

	for (FBoidData i : _neighbours)
	{
		cohesion += i.boidLocation;
	}

	cohesion /= _neighbours.Num();
	cohesion = cohesion - _boid.boidLocation;
	cohesion.Normalize();

	return cohesion * FlockManager->cohesionWeight;
}

FVector FFlocksManagerThread::CalculateSeparation(FBoidData _boid, TArray<FBoidData>& _neighbours)
{
	if (_neighbours.Num() == 0) { return FVector::ZeroVector; }

	FVector separation = FVector::ZeroVector;

	for (FBoidData i : _neighbours)
	{
		separation += _boid.boidLocation - i.boidLocation;
	}

	separation /= _neighbours.Num();
	separation *= 1.0f;
	separation.Normalize();

	return separation * FlockManager->separationWeight;
}

FVector FFlocksManagerThread::CalculateAlignment(FBoidData _boid, TArray<FBoidData>& _neighbours)
{
	if (_neighbours.Num() == 0) { return FVector::ZeroVector; }

	FVector alignment = FVector::ZeroVector;

	for (FBoidData i : _neighbours)
	{
		alignment += i.boidVelocity;
	}

	alignment /= _neighbours.Num();
	alignment.Normalize();

	return alignment * FlockManager->alignmentWeight;
}

FVector FFlocksManagerThread::CalculateTowardsZero(FBoidData _boid)
{
	FVector toZero = FlockManager->zeroLocation - _boid.boidLocation;
	toZero.Normalize();

	return toZero * FlockManager->toZeroWeight;
}