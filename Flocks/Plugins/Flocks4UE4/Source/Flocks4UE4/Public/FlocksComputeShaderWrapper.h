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

	BoidState(int32 _instanceId, float _accelerationSpeed, float _maxVelocity, FVector _position, FVector _direction)
	{
		instanceId = _instanceId;
		Position[0] = _position.X; Position[1] = _position.Y; Position[2] = _position.Z;
		Direction[0] = _direction.X; Direction[1] = _direction.Y; Direction[2] = _direction.Z;
		Acceleration[0] = _direction.X; Acceleration[1] = _direction.Y; Acceleration[2] = _direction.Z;
		AccelerationSpeed = _accelerationSpeed;
		MaxVelocity = _maxVelocity;
	}

	int32 instanceId = 0;
	float AccelerationSpeed = 0.0f;
	float MaxVelocity = 0.0f;
	float Direction[3] = { 0, 0, 0 };
	float Position[3] = { 0, 0, 0 };
	float Acceleration[3] = { 0, 0, 0 };
};

struct FlocksVolumeData
{
	FlocksVolumeData()
	{

	}

	FlocksVolumeData(int32 _instanceId, FVector _position, float _innerRadius, float _outerRadius, float _falloff, int _volumeType)
	{
		InstanceId = _instanceId;
		Position[0] = _position.X; Position[1] = _position.Y; Position[2] = _position.Z;
		InnerRadius = _innerRadius;
		OuterRadius = _outerRadius;
		Falloff = _falloff;
		VolumeType = _volumeType;
	}

	int32 InstanceId = 0;
	float Position[3] = { 0, 0, 0 };
	float InnerRadius = 0.0f;
	float OuterRadius = 0.0f;
	float Falloff = 1.0f;
	int VolumeType = 0;
};

class FLOCKS4UE4_API FlocksComputeShaderWrapper
{
public:
	FlocksComputeShaderWrapper(int32 _numBoids, float _cohesionRadius, float _separationRadius, float _alignmentRadius, float _cohesion, float _separation, float _alignment, float _maxVelocity, float _acceleration, ERHIFeatureLevel::Type _featureLevel);
	~FlocksComputeShaderWrapper();

	void Execute(const TArray<BoidState> &_currentStates, const TArray<FlocksVolumeData>& _volumeData, float _deltaTime);
	TArray<BoidState> GetStates() { return m_states; }

protected:
	void ExecuteComputeShader(const TArray<BoidState> &_currentStates, const TArray<FlocksVolumeData>& _volumeData, float _deltaTime);
	void ExecuteInRenderThread(const TArray<BoidState> &_currentStates, const TArray<FlocksVolumeData>& _volumeData, TArray<BoidState> &_result);

private:
	TArray<BoidState> m_states;
	int32 m_threadNumGroupCount;

	FConstantParameters m_constants;
	FVariableParameters m_variables;
	ERHIFeatureLevel::Type m_featureLevel;
	FRenderCommandFence m_releaseResourcesFence;
};