// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "Flocks4UE4.h"
#include "RenderCommandFence.h"
#include "FlocksComputeShader.h"

struct BoidState 
{
	BoidState()
	{

	}

	BoidState(int32 _instanceId, FVector _position, FVector _direction)
	{
		instanceId = _instanceId;
		Position[0] = _position.X; Position[1] = _position.Y; Position[2] = _position.Z;
		Direction[0] = _direction.X; Direction[1] = _direction.Y; Direction[2] = _direction.Z;
		Acceleration[0] = _direction.X; Acceleration[1] = _direction.Y; Acceleration[2] = _direction.Z;
	}

	int32 instanceId = 0;
	float Direction[3] = { 0, 0, 0 };
	float Position[3] = { 0, 0, 0 };
	float Acceleration[3] = { 0, 0, 0 };
};

class FLOCKS4UE4_API FlocksComputeShaderWrapper
{
public:
	FlocksComputeShaderWrapper(int32 _numBoids, float _cohesionRadius, float _separationRadius, float _alignmentRadius, float _cohesion, float _separation, float _alignment, float _maxVelocity, float _acceleration, ERHIFeatureLevel::Type _featureLevel);
	~FlocksComputeShaderWrapper();

	void Execute(const TArray<BoidState> &_currentStates, float _deltaTime);
	TArray<BoidState> GetStates() { return m_states; }

protected:
	void ExecuteComputeShader(const TArray<BoidState> &_currentStates, float _deltaTime);
	void ExecuteInRenderThread(const TArray<BoidState> &_currentStates, TArray<BoidState> &_result);

private:
	TArray<BoidState> m_states;
	int32 m_threadNumGroupCount;

	FConstantParameters m_constants;
	FVariableParameters m_variables;
	ERHIFeatureLevel::Type m_featureLevel;
	FRenderCommandFence m_releaseResourcesFence;
};