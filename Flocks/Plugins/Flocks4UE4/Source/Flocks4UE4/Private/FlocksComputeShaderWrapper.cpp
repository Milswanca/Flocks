// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FlocksComputeShaderWrapper.h"
#include "CoreUObject.h"
#include "Engine.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 32 //This has to be the same as in the compute shader's spec [X, X, 1]


FlocksComputeShaderWrapper::FlocksComputeShaderWrapper(int32 _numBoids, float _cohesionRadius, float _separationRadius, float _alignmentRadius, float _cohesion, float _separation, float _alignment, float _maxVelocity, float _accleration, ERHIFeatureLevel::Type _featureLevel)
{
	m_featureLevel = _featureLevel;
	m_constants.NumBoids = _numBoids;
	m_constants.CohesionRadius = _cohesionRadius;
	m_constants.SeparationRadius = _separationRadius;
	m_constants.AlignmentRadius = _alignmentRadius;
	m_constants.Cohesion = _cohesion;
	m_constants.Separation = _separation;
	m_constants.Alignment = _alignment;
	m_constants.Acceleration = _accleration;
	m_constants.MaxVelocity = _maxVelocity;
	m_constants.CalculationsPerThread = 1;

	m_variables = FVariableParameters();

	for (int i = 0; i < _numBoids; i++) {
		m_states.Add(BoidState());
	}

	m_threadNumGroupCount = (m_constants.NumBoids % (NUM_THREADS_PER_GROUP_DIMENSION * m_constants.CalculationsPerThread) == 0 ?
		m_constants.NumBoids / (NUM_THREADS_PER_GROUP_DIMENSION * m_constants.CalculationsPerThread) :
		m_constants.NumBoids / (NUM_THREADS_PER_GROUP_DIMENSION * m_constants.CalculationsPerThread) + 1);
	m_threadNumGroupCount = m_threadNumGroupCount == 0 ? 1 : m_threadNumGroupCount;
}

FlocksComputeShaderWrapper::~FlocksComputeShaderWrapper()
{
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
	m_constants.NumBoids = _currentStates.Num();
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

	TResourceArray<BoidState> data;
	for (int i = 0; i < m_constants.NumBoids; i++) {
		data.Add(_currentStates[i]);
	}
	for (int i = 0; i < m_constants.NumBoids; i++) {
		data.Add(_currentStates[i]);
	}

	TResourceArray<FlocksVolumeData> volumes;
	for (int i = 0; i < _volumeData.Num(); i++) {
		volumes.Add(_volumeData[i]);
	}

	FRHIResourceCreateInfo boidDataResource;
	boidDataResource.ResourceArray = &data;
	FStructuredBufferRHIRef boidsBuffer = RHICreateStructuredBuffer(sizeof(BoidState), sizeof(BoidState) * 2 * m_constants.NumBoids, BUF_UnorderedAccess | 0, boidDataResource);
	FUnorderedAccessViewRHIRef boidsUav = RHICreateUnorderedAccessView(boidsBuffer, false, false);

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
	shader->SetShaderData(commandList, boidsUav, volumesUav);
	shader->SetBuffers(commandList, m_constants, m_variables);
	DispatchComputeShader(commandList, *shader, 1, m_threadNumGroupCount, 1);
	shader->CleanupShaderData(commandList);

	char* shaderData = (char*)commandList.LockStructuredBuffer(boidsBuffer, 0, sizeof(BoidState) * 2 * m_constants.NumBoids, EResourceLockMode::RLM_ReadOnly);
	BoidState* p = (BoidState*)shaderData;
	for (int32 Row = 0; Row < m_constants.NumBoids; ++Row) {
		_result[Row] = *p;
		p++;
	}
	commandList.UnlockStructuredBuffer(boidsBuffer);
}