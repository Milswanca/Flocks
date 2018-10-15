// Fill out your copyright notice in the Description page of Project Settings.

#include "FlocksManager.h"
#include "FlocksAIController.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "FlocksManagerThread.h"
#include "FlocksVolume.h"
#include "FlocksRestrictionVolume.h"
#include "FlocksComputeShaderWrapper.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "FlocksInstancedMeshActor.h"
#include "NavigationSystem.h"

int32 AFlocksManager::currentGroupID = 0;

AFlocksManager::AFlocksManager()
{
	PrimaryActorTick.bCanEverTick = true;

	currentGroupID = 0;
}

void AFlocksManager::BeginPlay()
{
	Super::BeginPlay();

	for (int j = 0; j < SpawnGroup.SpawnData.Num(); ++j)
	{
		SpawnGroup.SpawnData[j].GroupData.GroupID = currentGroupID;
		currentGroupID++;

		m_groups.Add(SpawnGroup.SpawnData[j]);
	}

	for (int i = 0; i < m_groups.Num(); ++i)
	{
		FSpawnData data = m_groups[i];

		if (data.bUseInstancedMesh)
		{
			FActorSpawnParameters params;
			m_groups[i].InstancedMesh = GetWorld()->SpawnActor<AFlocksInstancedMeshActor>(GetActorLocation(), FRotator::ZeroRotator);
			m_groups[i].InstancedMesh->GetInstancedMesh()->SetStaticMesh(m_groups[i].FishMesh);
		}
	}

	for (int i = 0; i < m_groups.Num(); ++i)
	{
		FSpawnData data = m_groups[i];

		for (int k = 0; k < data.NumSpawns; ++k)
		{
			int32 id = -1;

			if (data.bUseInstancedMesh)
			{
				FTransform t;
				t.SetScale3D(m_groups[i].Scale);
				id = data.InstancedMesh->AddInstance(t);
			}
			else
			{
				AFlocksAIController* controller = GetWorld()->SpawnActorDeferred<AFlocksAIController>(m_groups[i].ControllerClass, FTransform());
				controller->SetID(m_boidData.Num(), this);
				m_groups[i].FlocksControllers.Add(controller);
				controller->FinishSpawning(FTransform());
			}

			float rand = FMath::RandRange(data.MinSpeed, data.MaxSpeed);
			m_boidData.Add(BoidState(m_boidData.Num(), i, data.GroupData.GroupID, id, rand, rand, FMath::VRand() * m_groups[i].SpawnRadius + GetActorLocation(), FMath::VRand()));
			m_groups[i].FishIDs.Add(m_boidData.Num() - 1);
		}
	}

	if (ComputeType == ComputeType::CT_GPU)
	{
		TArray<FGroupData> groupData;

		for (int i = 0; i < m_groups.Num(); ++i)
		{
			FSpawnData data = m_groups[i];

			groupData.Add(data.GroupData);
		}
		FShaderData shaderData;
		shaderData.NumBoids = m_boidData.Num();

		m_flockShader = MakeShared<FlocksComputeShaderWrapper>(shaderData, groupData, GetWorld()->Scene->GetFeatureLevel());
	}
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

void AFlocksManager::FlockVolumeMoved(int32 _volumeID, FVector _newPosition)
{
	m_volumeData[_volumeID].Position[0] = _newPosition.X;
	m_volumeData[_volumeID].Position[1] = _newPosition.Y;
	m_volumeData[_volumeID].Position[2] = _newPosition.Z;
}

void AFlocksManager::DeregisterBoid(int32 _id)
{
	m_boidData.RemoveAt(_id);
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

void AFlocksManager::Tick(float _deltaTime)
{
	switch (ComputeType)
	{
	case ComputeType::CT_CPU:
		UpdateCPU(_deltaTime);
		break;

	case ComputeType::CT_GPU:
		UpdateGPU(_deltaTime);
		break;
	}
}

void AFlocksManager::UpdateGPU(float _deltaTime)
{
	if (m_flockShader)
	{
		m_flockShader->Execute(m_boidData, m_volumeData, _deltaTime);
		TArray<BoidState> previousState = m_boidData;
		m_boidData = m_flockShader->GetStates();

		UpdateFromStates(_deltaTime);
	}
}

void AFlocksManager::UpdateCPU(float _deltaTime)
{
	//Create a copy
	TArray<BoidState> newStates = m_boidData;

	// Go Through the boids
	for (int i = 0; i < newStates.Num(); ++i)
	{
		BoidState currentState = newStates[i];
		FVector myPos = currentState.Position;
		FGroupData myGroup = m_groups[currentState.GroupIdLocal].GroupData;

		FVector steerCohesion = { 0.0f, 0.0f, 0.0f };
		FVector steerSeparation = { 0.0f, 0.0f, 0.0f };
		FVector steerAlignment = { 0.0f, 0.0f, 0.0f };
		FVector steerFlee = { 0.0f, 0.0f, 0.0f };
		FVector steerGoal = { 0.0f, 0.0f, 0.0f };

		float steerCohesionCnt = 0.0f;
		float steerSeparationCnt = 0.0f;
		float steerAlignmentCnt = 0.0f;
		float steerFleeCnt = 0.0f;
		float steerGoalCnt = 0.0f;

		// Go through the other boids
		for (int j = 0; j < m_boidData.Num(); ++j)
		{
			// If its not me
			if (currentState.instanceId != m_boidData[j].instanceId)
			{
				FVector theirPos = m_boidData[j].Position;

				float d = FVector::Distance(theirPos, myPos);
				if (d < myGroup.CohesionRadius)
				{
					steerCohesion += theirPos;
					steerCohesionCnt++;
				}
				if (d < myGroup.SeparationRadius)
				{
					steerSeparation += theirPos - myPos;
					steerSeparationCnt++;
				}
				if (d < myGroup.AlignmentRadius)
				{
					steerAlignment += m_boidData[j].Direction;
					steerAlignmentCnt++;
				}
			}
		}

		float closestDist = 100000000;
		int closestRest = -1;

		for (int j = 0; j < m_volumeData.Num(); ++j)
		{
			FVector volumePos = m_volumeData[j].Position;

			switch (m_volumeData[j].VolumeType)
			{
			case 0:
			{
				float d = FVector::Distance(volumePos, myPos);
				if (d < closestDist)
				{
					closestDist = d;
					closestRest = j;
				}
			}
			break;

			case 1:
			{
				float inf = AFlocksVolume::GetInfluence(myPos, volumePos, m_volumeData[j].InnerRadius, m_volumeData[j].OuterRadius, m_volumeData[j].Falloff);

				if (inf > 0.0f)
				{
					steerFlee -= (volumePos - myPos) * inf;
					steerFleeCnt++;
				}
			}
			break;

			case 2:
			{
				float inf = 1.0f - AFlocksVolume::GetInfluence(myPos, volumePos, m_volumeData[j].InnerRadius, m_volumeData[j].OuterRadius, m_volumeData[j].Falloff);

				if (inf > 0.0f)
				{
					steerGoal += (volumePos - myPos) * inf;
					steerGoalCnt++;
				}
			}
			break;
			}
		}

		FVector steerRestriction = FVector(0, 0, 0);
		if (closestRest > -1 && closestDist > m_volumeData[closestRest].OuterRadius)
		{
			steerRestriction = m_volumeData[closestRest].Position - myPos;
			steerRestriction.Normalize();
		}

		if (steerFleeCnt != 0)
		{
			steerFlee /= steerFleeCnt;
			steerFlee.Normalize();
		}

		if (steerGoalCnt != 0)
		{
			steerGoal /= steerGoalCnt;
			steerGoal.Normalize();
		}

		if (steerCohesionCnt != 0) {
			steerCohesion = (steerCohesion / steerCohesionCnt - myPos);
			steerCohesion.Normalize();
		}

		if (steerSeparationCnt != 0) {
			steerSeparation = -1.f * (steerSeparation / steerSeparationCnt);
			steerSeparation.Normalize();
		}

		if (steerAlignmentCnt != 0) {
			steerAlignment /= steerAlignmentCnt;
			steerAlignment.Normalize();
		}

		currentState.Acceleration = (steerGoal * myGroup.Goal + steerFlee * myGroup.Flee + steerCohesion * myGroup.Cohesion + steerSeparation * myGroup.Separation + steerAlignment * myGroup.Alignment + steerRestriction * myGroup.Restriction) * currentState.AccelerationSpeed;

		FVector direction = currentState.Direction + currentState.Acceleration * _deltaTime;
		currentState.Direction = direction.GetClampedToMaxSize(currentState.MaxVelocity);

		currentState.Position += currentState.Direction * _deltaTime;
		newStates[i] = currentState;
	}

	m_boidData = newStates;

	UpdateFromStates(_deltaTime);
}

void AFlocksManager::UpdateFromStates(float _deltaTime)
{
	for (int i = 0; i < m_groups.Num(); i++)
	{
		if (m_groups[i].bUseInstancedMesh)
		{
			AFlocksInstancedMeshActor* mesh = m_groups[i].InstancedMesh;

			for (int j = 0; j < m_groups[i].FishIDs.Num(); ++j)
			{
				BoidState data = m_boidData[m_groups[i].FishIDs[j]];

				if (FlockType == FlockType::FT_Herding)
				{
					data.Position = UNavigationSystemV1::ProjectPointToNavigation(this, data.Position);
					data.Direction.Y = 0.0f;
				}

				FVector direction = FVector(data.Direction[0], data.Direction[1], data.Direction[2]);
				direction.Normalize();
				FRotator rotation = FRotationMatrix::MakeFromX(data.Direction).Rotator() + m_groups[i].RotOffset;

				FTransform trans;
				mesh->SetLocationAndRotation(data.StaticMeshId, data.Position, rotation);

				m_boidData[m_groups[i].FishIDs[j]] = data;
			}

			mesh->GetInstancedMesh()->MarkRenderStateDirty();
		}
		else
		{
			for (int j = 0; j < m_groups[i].FlocksControllers.Num(); ++j)
			{
				AFlocksAIController* controller = m_groups[i].FlocksControllers[j];
				BoidState data = m_boidData[controller->GetID()];

				FVector direction = FVector(data.Direction[0], data.Direction[1], data.Direction[2]);
				direction.Normalize();
				FRotator rotation = FRotationMatrix::MakeFromX(direction).Rotator() + m_groups[i].RotOffset;

				if (FlockType == FlockType::FT_Herding)
				{
					data.Position = UNavigationSystemV1::ProjectPointToNavigation(this, data.Position);

					rotation = FRotationMatrix::MakeFromX(direction).Rotator();
					rotation.Pitch = 0.0f;
					rotation.Roll = 0.0f;
				}

				FTransform trans;
				controller->GetPawn()->SetActorLocationAndRotation(data.Position, rotation);

				m_boidData[controller->GetID()] = data;
			}
		}
	}
}

FVector AFlocksManager::GetBoidPosition(int32 _boidID)
{
	if (!m_boidData.IsValidIndex(_boidID)) { return FVector::ZeroVector; }

	BoidState f = m_boidData[_boidID];
	return FVector(f.Position[0], f.Position[1], f.Position[2]);
}