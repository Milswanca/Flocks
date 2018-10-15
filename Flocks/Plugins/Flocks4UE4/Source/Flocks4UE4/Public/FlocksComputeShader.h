// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

static const int32 GMaxNumFlockGroups = 20;

BEGIN_UNIFORM_BUFFER_STRUCT(FConstantParameters, )
UNIFORM_MEMBER(int, CalculationsPerThread)
END_UNIFORM_BUFFER_STRUCT(FConstantParameters)

BEGIN_UNIFORM_BUFFER_STRUCT(FVariableParameters, )
UNIFORM_MEMBER(float, DeltaSeconds)
UNIFORM_MEMBER(int, NumVolumes)
END_UNIFORM_BUFFER_STRUCT(FVariableParameters)

BEGIN_UNIFORM_BUFFER_STRUCT(FGroupDataParameters, )
UNIFORM_MEMBER_ARRAY(float, CohesionRadius, [GMaxNumFlockGroups])
UNIFORM_MEMBER_ARRAY(float, SeparationRadius, [GMaxNumFlockGroups])
UNIFORM_MEMBER_ARRAY(float, AlignmentRadius, [GMaxNumFlockGroups])
UNIFORM_MEMBER_ARRAY(float, Cohesion, [GMaxNumFlockGroups])
UNIFORM_MEMBER_ARRAY(float, Separation, [GMaxNumFlockGroups])
UNIFORM_MEMBER_ARRAY(float, Alignment, [GMaxNumFlockGroups])
UNIFORM_MEMBER_ARRAY(float, Restriction, [GMaxNumFlockGroups])
UNIFORM_MEMBER_ARRAY(float, Flee, [GMaxNumFlockGroups])
UNIFORM_MEMBER_ARRAY(float, Goal, [GMaxNumFlockGroups])
END_UNIFORM_BUFFER_STRUCT(FGroupDataParameters)

typedef TUniformBufferRef<FConstantParameters> FConstantParametersRef;
typedef TUniformBufferRef<FVariableParameters> FVariableParametersRef;
typedef TUniformBufferRef<FGroupDataParameters> FGroupDataParametersRef;

class FlocksComputeShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FlocksComputeShader, Global);
public:
	FlocksComputeShader() {}

	explicit FlocksComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
	
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& PermutationParams)
	{
		// Useful when adding a permutation of a particular shader
		return true;
	}

	static bool ShouldCache(EShaderPlatform Platform) { return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5); }
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParams = FGlobalShader::Serialize(Ar);

		Ar << BoidData;
		Ar << VolumeData;
		Ar << ShaderData;

		return bShaderHasOutdatedParams;
	}

	//This function is required to let us bind our runtime surface to the shader using an UAV.
	void SetShaderData(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef ShaderDataUAV, FUnorderedAccessViewRHIRef BoidDataUAV, FUnorderedAccessViewRHIRef VolumeDataUAV);
	//This function is required to bind our constant / uniform buffers to the shader.
	void SetBuffers(FRHICommandList& RHICmdList, FGroupDataParameters& GroupDataParamters, FConstantParameters& ConstantParameters, FVariableParameters& VariableParameters);
	//This is used to clean up the buffer binds after each invocation to let them be changed and used elsewhere if needed.
	void CleanupShaderData(FRHICommandList& RHICmdList);

private:
	//This is the actual output resource that we will bind to the compute shader
	FShaderResourceParameter BoidData;
	FShaderResourceParameter VolumeData;
	FShaderResourceParameter ShaderData;
};