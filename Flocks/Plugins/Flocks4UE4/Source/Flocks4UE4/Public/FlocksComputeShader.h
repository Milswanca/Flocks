// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

BEGIN_UNIFORM_BUFFER_STRUCT(FConstantParameters, )
UNIFORM_MEMBER(int, NumBoids)
UNIFORM_MEMBER(int, CalculationsPerThread)
UNIFORM_MEMBER(float, CohesionRadius)
UNIFORM_MEMBER(float, SeparationRadius)
UNIFORM_MEMBER(float, AlignmentRadius)
UNIFORM_MEMBER(float, Cohesion)
UNIFORM_MEMBER(float, Separation)
UNIFORM_MEMBER(float, Alignment)
UNIFORM_MEMBER(float, Acceleration)
UNIFORM_MEMBER(float, MaxVelocity)
END_UNIFORM_BUFFER_STRUCT(FConstantParameters)

BEGIN_UNIFORM_BUFFER_STRUCT(FVariableParameters, )
UNIFORM_MEMBER(float, DeltaSeconds)
UNIFORM_MEMBER(int, NumVolumes)
END_UNIFORM_BUFFER_STRUCT(FVariableParameters)

typedef TUniformBufferRef<FConstantParameters> FConstantParametersRef;
typedef TUniformBufferRef<FVariableParameters> FVariableParametersRef;

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

		return bShaderHasOutdatedParams;
	}

	//This function is required to let us bind our runtime surface to the shader using an UAV.
	void SetShaderData(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef BoidDataUAV, FUnorderedAccessViewRHIRef VolumeDataUAV);
	//This function is required to bind our constant / uniform buffers to the shader.
	void SetBuffers(FRHICommandList& RHICmdList, FConstantParameters& ConstantParameters, FVariableParameters& VariableParameters);
	//This is used to clean up the buffer binds after each invocation to let them be changed and used elsewhere if needed.
	void CleanupShaderData(FRHICommandList& RHICmdList);

private:
	//This is the actual output resource that we will bind to the compute shader
	FShaderResourceParameter BoidData;

	FShaderResourceParameter VolumeData;
};