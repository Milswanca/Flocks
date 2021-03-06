// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FlocksComputeShader.h"
#include "CoreUObject.h"
#include "Engine.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"

IMPLEMENT_UNIFORM_BUFFER_STRUCT(FConstantParameters, TEXT("FlocksConstant"))
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FVariableParameters, TEXT("FlocksVariable"))
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FGroupDataParameters, TEXT("FlocksGroups"))

FlocksComputeShader::FlocksComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FGlobalShader(Initializer)
{
	//This call is what lets the shader system know that the surface OutputSurface is going to be available in the shader. The second parameter is the name it will be known by in the shader
	BoidData.Bind(Initializer.ParameterMap, TEXT("data"));
	ShaderData.Bind(Initializer.ParameterMap, TEXT("flockData"));
	VolumeData.Bind(Initializer.ParameterMap, TEXT("volumeData"));
}

void FlocksComputeShader::ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
}

void FlocksComputeShader::SetShaderData(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef ShaderDataUAV, FUnorderedAccessViewRHIRef BoidDataUAV, FUnorderedAccessViewRHIRef VolumeDataUAV)
{
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();

	if (BoidData.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, BoidData.GetBaseIndex(), BoidDataUAV);

	if (VolumeData.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, VolumeData.GetBaseIndex(), VolumeDataUAV);

	if (ShaderData.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, ShaderData.GetBaseIndex(), ShaderDataUAV);
}

void FlocksComputeShader::SetBuffers(FRHICommandList& RHICmdList, FGroupDataParameters& GroupDataParamters, FConstantParameters& ConstantParameters, FVariableParameters& VariableParameters)
{
	FConstantParametersRef ConstantParametersBuffer;
	FVariableParametersRef VariableParametersBuffer;
	FGroupDataParametersRef GroupDataParametersBuffer;

	ConstantParametersBuffer = FConstantParametersRef::CreateUniformBufferImmediate(ConstantParameters, UniformBuffer_SingleDraw);
	VariableParametersBuffer = FVariableParametersRef::CreateUniformBufferImmediate(VariableParameters, UniformBuffer_SingleDraw);
	GroupDataParametersBuffer = FGroupDataParametersRef::CreateUniformBufferImmediate(GroupDataParamters, UniformBuffer_SingleDraw);

	SetUniformBufferParameter(RHICmdList, GetComputeShader(), GetUniformBufferParameter<FConstantParameters>(), ConstantParametersBuffer);
	SetUniformBufferParameter(RHICmdList, GetComputeShader(), GetUniformBufferParameter<FVariableParameters>(), VariableParametersBuffer);
	SetUniformBufferParameter(RHICmdList, GetComputeShader(), GetUniformBufferParameter<FGroupDataParameters>(), GroupDataParametersBuffer);
}

/* Unbinds buffers that will be used elsewhere */
void FlocksComputeShader::CleanupShaderData(FRHICommandList& RHICmdList)
{
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();

	if (BoidData.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, BoidData.GetBaseIndex(), FUnorderedAccessViewRHIRef());

	if (VolumeData.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, VolumeData.GetBaseIndex(), FUnorderedAccessViewRHIRef());

	if (ShaderData.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, ShaderData.GetBaseIndex(), FUnorderedAccessViewRHIRef());
}

IMPLEMENT_SHADER_TYPE(, FlocksComputeShader, TEXT("/Plugin/Flocks4UE4/Private/ComputeFishShader.usf"), TEXT("VS_test"), SF_Compute);

//IMPLEMENT_MODULE(FDefaultModuleImpl, ShaderFishPlugin)