// Fill out your copyright notice in the Description page of Project Settings.

#include "FlocksManager.h"
#include "FlocksAIController.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "FlocksManagerThread.h"
#include "FlocksVolume.h"
#include "FlocksRestrictionVolume.h"
#include "FlocksComputeShaderWrapper.h"

void AFlocksManager::BeginPlay()
{
	Super::BeginPlay();

	if (computeType == ComputeType::CT_GPU)
	{
		m_flockShader = MakeShared<FlocksComputeShaderWrapper>(1000, cohesionRadius, separationRadius, alignmentRadius, cohesionWeight, separationWeight, alignmentWeight, maxVelocity, maxAcceleration, GetWorld()->Scene->GetFeatureLevel());
	}

	//if (computeType == ComputeType::CT_MultiThreaded)
	//{
	//	for (int i = 0; i < numThreads; i++)
	//	{
	//		FThreadData data;
	//		m_threadData.Add(data);
	//		m_threadData[i].Thread = new FFlocksManagerThread(this, &m_boidData, &m_threadData[i].BoidsToCompute, &m_threadData[i].ComputedData);
	//	}
	//}
}

void AFlocksManager::BoidChangedAcceleration(int32 _id, float _acceleration)
{
	if (!m_boidData.IsValidIndex(_id)) { return; }

	m_boidData[_id].AccelerationSpeed = _acceleration;
}

void AFlocksManager::BoidChangedMaxVelocity(int32 _id, float _maxVelocity)
{
	if (!m_boidData.IsValidIndex(_id)) { return; }

	m_boidData[_id].MaxVelocity = _maxVelocity;
}

int32 AFlocksManager::RegisterBoid(AFlocksAIController* _controller)
{
	if (!_controller || !_controller->GetPawn()) { return -1; }

	m_boids.AddUnique(_controller);
	
	int32 id = m_boids.Num() - 1;
	m_boidData.Add(BoidState(id, _controller->GetAcceleration(), _controller->GetMaxVelocity(), _controller->GetPawn()->GetActorLocation(), _controller->GetDirection()));

	//if (computeType == ComputeType::CT_MultiThreaded)
	//{
	//	m_boidData.Add(FBoidData(m_boids.Num() - 1, _controller->GetPawn()->GetActorLocation(), _controller->GetDirection(), _controller->GetNeighbourDistanceSqr()));
	//	m_threadData[m_currentThread].BoidsToCompute.Add(m_boids.Num() - 1);
	//
	//	m_currentThread++;
	//	if (m_currentThread > numThreads - 1)
	//	{
	//		m_currentThread = 0;
	//	}
	//}

	return id;
}

void AFlocksManager::DeregisterBoid(AFlocksAIController* _controller)
{
	m_boids.Remove(_controller);
}

int32 AFlocksManager::RegisterVolume(class AFlocksVolume* _volume)
{
	m_flockVolumes.AddUnique(_volume);

	int32 id = m_flockVolumes.Num() - 1;
	m_volumeData.Add(FlocksVolumeData(id, _volume->GetActorLocation(), _volume->GetInnerRadius(), _volume->GetOuterRadius(), _volume->GetFalloff(), (int32)_volume->GetVolumeType()));

	return m_flockVolumes.Num() - 1;
}

void AFlocksManager::DeregisterVolume(class AFlocksVolume* _volume)
{
	m_flockVolumes.Remove(_volume);
}

void AFlocksManager::UpdateBoids(float _deltaTime)
{
	switch (computeType)
	{
	case ComputeType::CT_CPU:
		for (AFlocksAIController* i : m_boids)
		{
			TArray<AFlocksAIController*> neighbours;
			CalculateNeighbours(i, neighbours);

			FVector finalDir = i->GetDirection() + CalculateCohesion(i, neighbours) + CalculateAlignment(i, neighbours) + CalculateSeparation(i, neighbours) + CalculateTowardsZero(i) + CalculateFlee(i) + CalculateRestriction(i);
			finalDir.Normalize();

			i->SetDirection(finalDir);
		}
		break;

	//case ComputeType::CT_MultiThreaded:
	//	for (int i = 0; i < m_threadData.Num(); ++i)
	//	{
	//		if (m_threadData[i].Thread && m_threadData[i].Thread->IsFinished())
	//		{
	//			for (FBoidComputedData j : m_threadData[i].ComputedData)
	//			{
	//				m_boids[j.boidID]->SetDirection(j.direction);
	//				m_boidData[j.boidID] = FBoidData(j.boidID, m_boids[j.boidID]->GetPawn()->GetActorLocation(), m_boids[j.boidID]->GetDirection(), m_boids[j.boidID]->GetNeighbourDistanceSqr());
	//			}
	//
	//			m_threadData[i].ComputedData.Reset();
	//		}
	//	}
	//	break;

	case ComputeType::CT_GPU:
		if (GetWorld()->GetTimeSeconds() > 3.0f)
		{
			if (m_flockShader)
			{
				m_flockShader->Execute(m_boidData, m_volumeData, _deltaTime);
				TArray<BoidState> previousState = m_boidData;
				m_boidData = m_flockShader->GetStates();

				for (int i = 0; i < m_boids.Num(); i++)
				{
					if (!m_boids[i]) { continue; }

					m_boids[i]->GetPawn()->SetActorLocation(FVector(m_boidData[i].Position[0], m_boidData[i].Position[1], m_boidData[i].Position[2]));
					m_boids[i]->SetDirection(FVector(m_boidData[i].Direction[0], m_boidData[i].Direction[1], m_boidData[i].Direction[2]));
				}
			}
		}
		break;
	}
}

void AFlocksManager::CalculateNeighbours(AFlocksAIController* _controller, TArray<AFlocksAIController*>& _outNeighbours)
{
	float nDist = _controller->GetNeighbourDistanceSqr();

	for (AFlocksAIController* i : m_boids)
	{
		if (i == _controller) { continue; }

		float dist = FVector::DistSquared(i->GetPawn()->GetActorLocation(), _controller->GetPawn()->GetActorLocation());

		if (dist < nDist)
		{
			_outNeighbours.Add(i);
		}
	}
}

FVector AFlocksManager::CalculateCohesion(class AFlocksAIController* _controller, TArray<class AFlocksAIController*> _neighbours)
{
	if (_neighbours.Num() == 0) { return FVector::ZeroVector; }

	FVector cohesion = FVector::ZeroVector;

	for (AFlocksAIController* i : _neighbours)
	{
		cohesion += i->GetPawn()->GetActorLocation();
	}

	cohesion /= _neighbours.Num();
	cohesion = cohesion - _controller->GetPawn()->GetActorLocation();
	cohesion.Normalize();

	return cohesion * cohesionWeight;
}

FVector AFlocksManager::CalculateSeparation(class AFlocksAIController* _controller, TArray<class AFlocksAIController*> _neighbours)
{
	if (_neighbours.Num() == 0) { return FVector::ZeroVector; }

	FVector separation = FVector::ZeroVector;

	for (AFlocksAIController* i : _neighbours)
	{
		separation += _controller->GetPawn()->GetActorLocation() - i->GetPawn()->GetActorLocation();
	}

	separation /= _neighbours.Num();
	separation *= 1.0f;
	separation.Normalize();

	return separation * separationWeight;
}

FVector AFlocksManager::CalculateAlignment(class AFlocksAIController* _controller, TArray<class AFlocksAIController*> _neighbours)
{
	if (_neighbours.Num() == 0) { return FVector::ZeroVector; }

	FVector alignment = FVector::ZeroVector;

	for (AFlocksAIController* i : _neighbours)
	{
		alignment += i->GetDirection();
	}

	alignment /= _neighbours.Num();
	alignment.Normalize();

	return alignment * alignmentWeight;
}

FVector AFlocksManager::CalculateTowardsZero(class AFlocksAIController* _controller)
{
	FVector toZero = zeroLocation - _controller->GetPawn()->GetActorLocation();
	toZero.Normalize();

	return toZero * toZeroWeight;
}

FVector AFlocksManager::CalculateFlee(class AFlocksAIController* _controller)
{
	if (m_flockVolumes.Num() <= 0) { return FVector::ZeroVector; }

	FVector flee = FVector::ZeroVector;
	int32 numFlees = 0;

	for (AFlocksVolume* volume : m_flockVolumes)
	{
		if (volume->GetVolumeType() != EVolumeType::VT_Flee) { continue; }

		FVector newFlee = _controller->GetPawn()->GetActorLocation() - volume->GetActorLocation();
		newFlee.Normalize();
		newFlee *= volume->GetInfluence(_controller->GetPawn()->GetActorLocation());
		numFlees++;

		flee += newFlee;
	}

	if (numFlees == 0)
	{
		return FVector::ZeroVector;
	}

	flee /= numFlees;

	if (flee.SizeSquared() < 0.1f)
	{
		return FVector::ZeroVector;
	}

	flee.Normalize();

	return flee * fleeWeight;
}

FVector AFlocksManager::CalculateRestriction(class AFlocksAIController* _controller)
{
	if (m_flockVolumes.Num() <= 0) { return FVector::ZeroVector; }

	FVector restriction = FVector::ZeroVector;
	int32 numRestrictions = 0;

	for (AFlocksVolume* volume : m_flockVolumes)
	{
		if (volume->GetVolumeType() != EVolumeType::VT_Restriction) { continue; }

		FVector newRest = _controller->GetPawn()->GetActorLocation() - volume->GetActorLocation();
		newRest.Normalize();
		newRest *= volume->GetInfluence(_controller->GetPawn()->GetActorLocation());
		numRestrictions++;

		restriction += newRest;
	}

	if (numRestrictions == 0)
	{
		return FVector::ZeroVector;
	}

	restriction /= numRestrictions;

	if (restriction.SizeSquared() < 0.1f)
	{
		return FVector::ZeroVector;
	}

	restriction.Normalize();
	return restriction * restrictionWeight;
}

//Killing the thread for example in EndPlay() or BeginDestroy()
void AFlocksManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//if (computeType == ComputeType::CT_MultiThreaded)
	//{
	//	for (FThreadData i : m_threadData)
	//	{
	//		if (i.Thread)
	//		{
	//			i.Thread->EnsureCompletion();
	//			delete i.Thread;
	//			i.Thread = nullptr;
	//		}
	//	}
	//
	//	m_threadData.Reset();
	//}

	Super::EndPlay(EndPlayReason);
}

void AFlocksManager::BeginDestroy()
{
	//if (computeType == ComputeType::CT_MultiThreaded)
	//{
	//	for (FThreadData i : m_threadData)
	//	{
	//		if (i.Thread)
	//		{
	//			i.Thread->EnsureCompletion();
	//			delete i.Thread;
	//			i.Thread = nullptr;
	//		}
	//	}
	//
	//	m_threadData.Reset();
	//}

	Super::BeginDestroy();
}

void AFlocksManager::GetBoids(TArray<class AFlocksAIController *>& _outBoids)
{
	_outBoids = m_boids;
}