// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "Flocks4UE4.h"
#include "RenderCommandFence.h"
#include "FlocksComputeShader.h"
#include "FlocksComputeShaderWrapper.generated.h"

struct BoidState
{
public:
	BoidState()
	{

	}

	BoidState(int32 _instanceId, int32 _boidGroupLocal, int32 _boidGroupGlobal, int32 _staticMeshId, float _accelerationSpeed, float _maxVelocity, FVector _position, FVector _direction)
	{
		instanceId = _instanceId;
		GroupIdGlobal = _boidGroupGlobal;
		GroupIdLocal = _boidGroupLocal;
		StaticMeshId = _staticMeshId;
		Position = _position;
		Direction = _direction;
		Acceleration = _direction;
		AccelerationSpeed = _accelerationSpeed;
		MaxVelocity = _maxVelocity;
	}

	int32 instanceId = 0;
	int32 GroupIdLocal = 0;
	int32 GroupIdGlobal = 0;
	int32 StaticMeshId = -1;
	float AccelerationSpeed = 0.0f;
	float MaxVelocity = 0.0f;
	FVector Direction = FVector(0, 0, 0);
	FVector Position = FVector(0, 0, 0);
	FVector Acceleration = FVector(0, 0, 0);
};

struct FlocksVolumeData
{
public:
	FlocksVolumeData()
	{

	}

	FlocksVolumeData(int32 _instanceId, FVector _position, float _innerRadius, float _outerRadius, float _falloff, int _volumeType)
	{
		InstanceId = _instanceId;
		Position = _position;
		InnerRadius = _innerRadius;
		OuterRadius = _outerRadius;
		Falloff = _falloff;
		VolumeType = _volumeType;
	}

	int32 InstanceId = 0;
	FVector Position = FVector(0, 0, 0);
	float InnerRadius = 0.0f;
	float OuterRadius = 0.0f;
	float Falloff = 1.0f;
	int VolumeType = 0;
};

struct FShaderData
{
public:
	FShaderData()
	{

	}

	FShaderData(int32 _numBoids)
	{
		NumBoids = _numBoids;
	}

	int32 NumBoids;
};

USTRUCT(BlueprintType)
struct FGroupData
{
	GENERATED_USTRUCT_BODY()

	FGroupData() { }

	FGroupData(int32 _groupID, float _cohesionRadius, float _separationRadius, float _alignmentRadius, float _cohesion, float _separation, float _alignment, float _restriction, float _flee, float _goal, float _acceleration, float _maxVelocity)
	{
		GroupID = _groupID;
		CohesionRadius = _cohesionRadius;
		SeparationRadius = _separationRadius;
		AlignmentRadius = _alignmentRadius;
		Cohesion = _cohesion;
		Alignment = _alignment;
		Separation = _separation;
		Restriction = _restriction;
		Flee = _flee;
		Goal = _goal;
		Acceleration = _acceleration;
		MaxVelocity = _maxVelocity;
	}

	UPROPERTY()
		int32 GroupID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
	float CohesionRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
	float SeparationRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
	float AlignmentRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
	float Cohesion;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
	float Separation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
	float Alignment;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
	float Restriction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
	float Flee;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
	float Goal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
	float Acceleration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
	float MaxVelocity;
};

class FLOCKS4UE4_API FlocksComputeShaderWrapper
{
public:
	FlocksComputeShaderWrapper(FShaderData _shaderData, const TArray<FGroupData> _groupData, ERHIFeatureLevel::Type _featureLevel);
	~FlocksComputeShaderWrapper();

	void SetShaderData(FShaderData _shaderData);

	void Execute(const TArray<BoidState> &_currentStates, const TArray<FlocksVolumeData>& _volumeData, float _deltaTime);
	TArray<BoidState> GetStates() { return m_states; }

protected:
	void ExecuteComputeShader(const TArray<BoidState> &_currentStates, const TArray<FlocksVolumeData>& _volumeData, float _deltaTime);
	void ExecuteInRenderThread(const TArray<BoidState> &_currentStates, const TArray<FlocksVolumeData>& _volumeData, TArray<BoidState> &_result);

private:

	FShaderData m_shaderData;
	TArray<BoidState> m_states;
	int32 m_threadNumGroupCount;

	FConstantParameters m_constants;
	FVariableParameters m_variables;
	FGroupDataParameters m_groupData;
	ERHIFeatureLevel::Type m_featureLevel;
	FRenderCommandFence m_releaseResourcesFence;
};