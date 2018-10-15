// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FlocksComputeShaderWrapper.h"
#include "CoreUObject.h"
#include "Engine.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 32 //This has to be the same as in the compute shader's spec [X, X, 1]


FlocksComputeShaderWrapper::FlocksComputeShaderWrapper(FShaderData _shaderData, const TArray<FGroupData> _groupData, ERHIFeatureLevel::Type _featureLevel)
{
	m_featureLevel = _featureLevel;
	m_constants.CalculationsPerThread = 1;

	for (int i = 0; i < _groupData.Num(); ++i)
	{
		int32 groupID = _groupData[i].GroupID;
		m_groupData.Alignment[groupID] = _groupData[i].Alignment;
		m_groupData.AlignmentRadius[groupID] = _groupData[i].AlignmentRadius;
		m_groupData.Cohesion[groupID] = _groupData[i].Cohesion;
		m_groupData.CohesionRadius[groupID] = _groupData[i].CohesionRadius;
		m_groupData.Flee[groupID] = _groupData[i].Flee;
		m_groupData.Goal[groupID] = _groupData[i].Goal;
		m_groupData.Restriction[groupID] = _groupData[i].Restriction;
		m_groupData.Separation[groupID] = _groupData[i].Separation;
		m_groupData.SeparationRadius[groupID] = _groupData[i].SeparationRadius;
	}

	m_shaderData = _shaderData;

	m_threadNumGroupCount = (m_shaderData.NumBoids % (NUM_THREADS_PER_GROUP_DIMENSION * m_constants.CalculationsPerThread) == 0 ?
		m_shaderData.NumBoids / (NUM_THREADS_PER_GROUP_DIMENSION * m_constants.CalculationsPerThread) :
		m_shaderData.NumBoids / (NUM_THREADS_PER_GROUP_DIMENSION * m_constants.CalculationsPerThread) + 1);
	m_threadNumGroupCount = m_threadNumGroupCount == 0 ? 1 : m_threadNumGroupCount;

	m_variables = FVariableParameters();
}

FlocksComputeShaderWrapper::~FlocksComputeShaderWrapper()
{
}

void FlocksComputeShaderWrapper::SetShaderData(FShaderData _shaderData)
{
	m_shaderData = _shaderData;
}

void FlocksComputeShaderWrapper::Execute(const TArray<BoidState> &_currentStates, const TArray<FlocksVolumeData>& _volumeData, float _deltaTime)
{
	ExecuteComputeShader(_currentStates, _volumeData, _deltaTime);
	m_releaseResourcesFence.BeginFence();
	m_releaseResourcesFence.Wait();
}

void FlocksComputeShaderWrapper::ExecuteComputeShader(const TArray<BoidState> &_currentStates, const TArray<FlocksVolumeData>& _volumeData, float _deltaTime)
{
	m_variables.DeltaSeconds = _deltaTime;
	m_variables.NumVolumes = _volumeData.Num();
	m_shaderData.NumBoids = _currentStates.Num();

	ENQUEUE_UNIQUE_RENDER_COMMAND_FOURPARAMETER(FComputeShaderRunner,
		FlocksComputeShaderWrapper*, processing, this,
		TArray<BoidState>&, result, m_states,
		const TArray<FlocksVolumeData>&, volumes, _volumeData,
		const TArray<BoidState>&, states, _currentStates,
		{
			processing->ExecuteInRenderThread(states, volumes, result);
		}
	);
}

void FlocksComputeShaderWrapper::ExecuteInRenderThread(const TArray<BoidState> &_currentStates, const TArray<FlocksVolumeData>& _volumeData, TArray<BoidState> &_result)
{
	check(IsInRenderingThread());

	_result.SetNum(m_shaderData.NumBoids);

	if (m_shaderData.NumBoids == 0) { return; }

	TResourceArray<BoidState> data;
	for (int i = 0; i < m_shaderData.NumBoids; i++) {
		data.Add(_currentStates[i]);
	}
	for (int i = 0; i < m_shaderData.NumBoids; i++) {
		data.Add(_currentStates[i]);
	}

	TResourceArray<FlocksVolumeData> volumes;
	for (int i = 0; i < _volumeData.Num(); i++) {
		volumes.Add(_volumeData[i]);
	}

	TResourceArray<FShaderData> shaderData;
	shaderData.Add(m_shaderData);

	FRHIResourceCreateInfo shaderDataResource;
	shaderDataResource.ResourceArray = &shaderData;
	FStructuredBufferRHIRef shaderDataBuffer = RHICreateStructuredBuffer(sizeof(FShaderData), sizeof(FShaderData), BUF_UnorderedAccess | 0, shaderDataResource);
	FUnorderedAccessViewRHIRef shaderUAV = RHICreateUnorderedAccessView(shaderDataBuffer, false, false);

	FRHIResourceCreateInfo boidDataResource;
	boidDataResource.ResourceArray = &data;
	FStructuredBufferRHIRef boidsBuffer;
	FUnorderedAccessViewRHIRef boidsUav;

	if (data.Num() > 0)
	{
		boidsBuffer = RHICreateStructuredBuffer(sizeof(BoidState), sizeof(BoidState) * 2 * m_shaderData.NumBoids, BUF_UnorderedAccess | 0, boidDataResource);
		boidsUav = RHICreateUnorderedAccessView(boidsBuffer, false, false);
	}

	FRHIResourceCreateInfo volumesResource;
	volumesResource.ResourceArray = &volumes;
	FStructuredBufferRHIRef volumesBuffer;
	FUnorderedAccessViewRHIRef volumesUav;

	if (volumes.Num() > 0)
	{
		volumesBuffer = RHICreateStructuredBuffer(sizeof(FlocksVolumeData), sizeof(FlocksVolumeData) * _volumeData.Num(), BUF_UnorderedAccess | 0, volumesResource);
		volumesUav = RHICreateUnorderedAccessView(volumesBuffer, false, false);
	}

	FRHICommandListImmediate& commandList = GRHICommandList.GetImmediateCommandList();
	TShaderMapRef<FlocksComputeShader> shader(GetGlobalShaderMap(m_featureLevel));
	commandList.SetComputeShader(shader->GetComputeShader());
	shader->SetShaderData(commandList, shaderUAV, boidsUav, volumesUav);
	shader->SetBuffers(commandList, m_groupData, m_constants, m_variables);
	DispatchComputeShader(commandList, *shader, 1, m_threadNumGroupCount, 1);
	shader->CleanupShaderData(commandList);

	char* outBoidData = (char*)commandList.LockStructuredBuffer(boidsBuffer, 0, sizeof(BoidState) * 2 * m_shaderData.NumBoids, EResourceLockMode::RLM_ReadOnly);
	BoidState* p = (BoidState*)outBoidData;
	for (int32 Row = 0; Row < m_shaderData.NumBoids; ++Row) {
		_result[Row] = *p;
		p++;
	}
	commandList.UnlockStructuredBuffer(boidsBuffer);
}