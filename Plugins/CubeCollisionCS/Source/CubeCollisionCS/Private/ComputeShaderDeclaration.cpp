#include"ComputeShaderDeclaration.h"
#include "CubeCollisionCS.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"
#include "RenderTargetPool.h"
#include "RHIUtilities.h"
#include "RHI.h"



#define NUM_THREADS_PER_GROUP_DIMENSION 32


/***************************************************************************/
/* This class is what encapsulates the shader in the engine.               */
/* It is the main bridge between the HLSL located in the engine directory  */
/* and the engine itself.                                                  */

class UpdateKernel : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(UpdateKernel)
	SHADER_USE_PARAMETER_STRUCT(UpdateKernel, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
			SHADER_PARAMETER_UAV(RWStructuredBuffer<float4>,Position )
			SHADER_PARAMETER_UAV(RWStructuredBuffer<float4>,Velocity)
			SHADER_PARAMETER_UAV(RWStructuredBuffer<float4>, Acc)
			SHADER_PARAMETER_UAV(RWStructuredBuffer<float4>, Force)
			SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, particleNumInGrid)
			SHADER_PARAMETER(float, m)
			SHADER_PARAMETER(float, dt)
			SHADER_PARAMETER(float, r)
			SHADER_PARAMETER(float, stiffness)
			SHADER_PARAMETER(int, grid_x)
			SHADER_PARAMETER(int, grid_y)
			SHADER_PARAMETER(int, grid_z)
			SHADER_PARAMETER(int, numOfParticle)
			SHADER_PARAMETER(float, grid_size)
			
    END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}

};
// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(UpdateKernel, "/CubeCollisionCS/Private/CubeCollision.usf", "update", SF_Compute);

/***************************************************************************/
//      update Zsum kernel   ***********************************************
/***************************************************************************/

class UpdateZSum : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(UpdateZSum)
	SHADER_USE_PARAMETER_STRUCT(UpdateZSum, FGlobalShader)

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, particleNumInGrid)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, zSum)
		SHADER_PARAMETER(int, grid_x)
		SHADER_PARAMETER(int, grid_y)
		SHADER_PARAMETER(int, grid_z)
		END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), 32);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), 32);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}

};
// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(UpdateZSum, "/CubeCollisionCS/Private/AssignP2G.usf", "updateZSum", SF_Compute);


/***************************************************************************/
//      test fill particle ID kernel   ***************************** ********
/***************************************************************************/

class FillparticleNumInGrid : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FillparticleNumInGrid)
	SHADER_USE_PARAMETER_STRUCT(FillparticleNumInGrid, FGlobalShader)

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, particleNumInGrid)
		SHADER_PARAMETER(int, grid_x)
		SHADER_PARAMETER(int, grid_y)
		SHADER_PARAMETER(int, grid_z)
		END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), 32);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), 32);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}

};
// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FillparticleNumInGrid, "/CubeCollisionCS/Private/AssignP2G.usf", "fillparticleNumInGrid", SF_Compute);


/***************************************************************************/
//      fill ZSumkernel                ***************************** ********
/***************************************************************************/
class FillZSum : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FillZSum)
	SHADER_USE_PARAMETER_STRUCT(FillZSum, FGlobalShader)

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, zSum)
		SHADER_PARAMETER(int, grid_x)
		SHADER_PARAMETER(int, grid_y)
		SHADER_PARAMETER(int, grid_z)
		END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), 32);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), 32);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}

};
// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FillZSum, "/CubeCollisionCS/Private/AssignP2G.usf", "fillZSum", SF_Compute);


/***************************************************************************/
//      fill prefix kernel                ***************************** ****
/***************************************************************************/

class InitPrefix : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(InitPrefix)
	SHADER_USE_PARAMETER_STRUCT(InitPrefix, FGlobalShader)

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, zSum)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, prefixSum)
		SHADER_PARAMETER(int, grid_x)
		SHADER_PARAMETER(int, grid_y)
		SHADER_PARAMETER(int, grid_z)
		END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), 1);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), 1);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}

};
// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(InitPrefix, "/CubeCollisionCS/Private/AssignP2G.usf", "initPrefix", SF_Compute);


/***************************************************************************/
//      fill lists kernel                ***************************** ****
/***************************************************************************/

class FillGridList : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FillGridList)
	SHADER_USE_PARAMETER_STRUCT(FillGridList, FGlobalShader)

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, particleNumInGrid)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, prefixSum)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, listHead)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, listCur)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, listTail)
		SHADER_PARAMETER(int, grid_x)
		SHADER_PARAMETER(int, grid_y)
		SHADER_PARAMETER(int, grid_z)
		END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), 32);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), 32);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}

};
// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FillGridList, "/CubeCollisionCS/Private/AssignP2G.usf", "fillGridList", SF_Compute);


/***************************************************************************/
//      fill particle kernel                ***************************** ****
/***************************************************************************/
class FillParticlieID : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FillParticlieID)
	SHADER_USE_PARAMETER_STRUCT(FillParticlieID, FGlobalShader)

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, particle_id)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<float4>, Position)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, listCur)
		SHADER_PARAMETER(int, grid_x)
		SHADER_PARAMETER(int, grid_y)
		SHADER_PARAMETER(int, grid_z)
		SHADER_PARAMETER(int, numOfParticle)
		SHADER_PARAMETER(float, grid_size)
		END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), 32);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), 32);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}

};
// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FillParticlieID, "/CubeCollisionCS/Private/AssignP2G.usf", "fillParticlieID", SF_Compute);



class Collision : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(Collision)
	SHADER_USE_PARAMETER_STRUCT(Collision, FGlobalShader)

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWStructuredBuffer<float4>, Position)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<float4>, Velocity)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<float4>, Force)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, listHead)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, listTail)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, particle_id)
		SHADER_PARAMETER(float, m)
		SHADER_PARAMETER(float, dt)
		SHADER_PARAMETER(float, r)
		SHADER_PARAMETER(float, stiffness)  
		SHADER_PARAMETER(float, restitution_coef)
		SHADER_PARAMETER(int, grid_x)
		SHADER_PARAMETER(int, grid_y)
		SHADER_PARAMETER(int, grid_z)
		SHADER_PARAMETER(int, numOfParticle)
		SHADER_PARAMETER(float, grid_size)

		END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}

};
// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(Collision, "/CubeCollisionCS/Private/CubeCollision.usf", "collision", SF_Compute);





void FCubeCollision::Execute_RenderThread(FRDGBuilder& builder, const FSceneTextures& SceneTextures)
{

	FRHICommandListImmediate& RHICmdList = builder.RHICmdList;

	//make sure that CachedParameters are updated
	//if (!bCachedParametersValid)
	//{
	//	return;
	//}

	check(IsInRenderingThread());




	for (int i = 0; i < 60; i++) {


		//*******************************************************************
		//        test fillParticleNUm kernel        ************************
		//*******************************************************************
		FillparticleNumInGrid::FParameters fillPassParameters;
		fillPassParameters.particleNumInGrid = particleNumInGridBufferUAV;
		fillPassParameters.grid_x = grid_x;
		fillPassParameters.grid_y = grid_y;
		fillPassParameters.grid_z = grid_z;
		//Get a reference to our shader type from global shader map
		TShaderMapRef<FillparticleNumInGrid> fillparticleNumInGrid(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		RHICmdList.BlockUntilGPUIdle();
		FComputeShaderUtils::Dispatch(RHICmdList, fillparticleNumInGrid, fillPassParameters,
			FIntVector(FMath::DivideAndRoundUp(50, NUM_THREADS_PER_GROUP_DIMENSION),
				FMath::DivideAndRoundUp(50, NUM_THREADS_PER_GROUP_DIMENSION), 20));


		
		//*******************************************************************
		//                update kernel              ************************
		//*******************************************************************

		UpdateKernel::FParameters updatePassParameters;
		updatePassParameters.Position = PositionBufferUAV;
		updatePassParameters.Velocity = VelocityBufferUAV;
		updatePassParameters.Force = ForceBufferUAV;
		updatePassParameters.Acc = AccBufferUAV;
		updatePassParameters.particleNumInGrid = particleNumInGridBufferUAV;


		updatePassParameters.dt = dt;
		updatePassParameters.m = m;
		updatePassParameters.r = r;
		updatePassParameters.stiffness = stiffness;
		updatePassParameters.grid_x = grid_x;
		updatePassParameters.grid_y = grid_y;
		updatePassParameters.grid_z = grid_z;
		updatePassParameters.grid_size = grid_size;
		updatePassParameters.numOfParticle = numOfCubes;
		//Get a reference to our shader type from global shader map
		TShaderMapRef<UpdateKernel> updateKernel(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		RHICmdList.BlockUntilGPUIdle();
		FComputeShaderUtils::Dispatch(RHICmdList, updateKernel, updatePassParameters,
			FIntVector(FMath::DivideAndRoundUp(32, NUM_THREADS_PER_GROUP_DIMENSION),
				FMath::DivideAndRoundUp(32, NUM_THREADS_PER_GROUP_DIMENSION), 1));
	
		
		//*******************************************************************
//        test fillZsum kernel        ************************
//*******************************************************************
		FillZSum::FParameters fillZSumPassParameters;
		fillZSumPassParameters.zSum = zSumBufferUAV;
		fillZSumPassParameters.grid_x = grid_x;
		fillZSumPassParameters.grid_y = grid_y;
		fillZSumPassParameters.grid_z = grid_z;

		//Get a reference to our shader type from global shader map
		TShaderMapRef<FillZSum> fillZSum(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		RHICmdList.BlockUntilGPUIdle();
		FComputeShaderUtils::Dispatch(RHICmdList, fillZSum, fillZSumPassParameters,
			FIntVector(FMath::DivideAndRoundUp(50, NUM_THREADS_PER_GROUP_DIMENSION),
				FMath::DivideAndRoundUp(50, NUM_THREADS_PER_GROUP_DIMENSION), 1));



		//*******************************************************************
		//                update zSum                ************************
		//*******************************************************************
		UpdateZSum::FParameters zSumPassParameters;

		zSumPassParameters.particleNumInGrid = particleNumInGridBufferUAV;
		zSumPassParameters.zSum = zSumBufferUAV;
		zSumPassParameters.grid_x = grid_x;
		zSumPassParameters.grid_y = grid_y;
		zSumPassParameters.grid_z = grid_z;

		TShaderMapRef<UpdateZSum> updateZSum(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		RHICmdList.BlockUntilGPUIdle();
		FComputeShaderUtils::Dispatch(RHICmdList, updateZSum, zSumPassParameters,
			FIntVector(FMath::DivideAndRoundUp(50, NUM_THREADS_PER_GROUP_DIMENSION),
				FMath::DivideAndRoundUp(50, NUM_THREADS_PER_GROUP_DIMENSION), 20));

		

		//*******************************************************************
		//                Init  prefixSum                ************************
		//*******************************************************************

		InitPrefix::FParameters initPrefixPassParameters;

		initPrefixPassParameters.prefixSum = prefixSumBufferUAV;
		initPrefixPassParameters.zSum = zSumBufferUAV;
		initPrefixPassParameters.grid_x = grid_x;
		initPrefixPassParameters.grid_y = grid_y;
		initPrefixPassParameters.grid_z = grid_z;

		TShaderMapRef<InitPrefix> initPrefix(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		RHICmdList.BlockUntilGPUIdle();
		FComputeShaderUtils::Dispatch(RHICmdList, initPrefix, initPrefixPassParameters,
			//FIntVector(FMath::DivideAndRoundUp(50, NUM_THREADS_PER_GROUP_DIMENSION),
			//	FMath::DivideAndRoundUp(50, NUM_THREADS_PER_GROUP_DIMENSION), 1));
			FIntVector(1, 1, 1)
			);

		//*******************************************************************
		//                fill grid list              ************************
		//*******************************************************************
		FillGridList::FParameters fillGridListPassParameters;

		fillGridListPassParameters.particleNumInGrid = particleNumInGridBufferUAV;
		fillGridListPassParameters.listHead = listHeadBufferUAV;
		fillGridListPassParameters.listCur = listCurBufferUAV;
		fillGridListPassParameters.listTail = listTailBufferUAV;
		fillGridListPassParameters.prefixSum = prefixSumBufferUAV;
		fillGridListPassParameters.grid_x = grid_x;
		fillGridListPassParameters.grid_y = grid_y;
		fillGridListPassParameters.grid_z = grid_z;

		TShaderMapRef<FillGridList> fillGridList(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		RHICmdList.BlockUntilGPUIdle();
		FComputeShaderUtils::Dispatch(RHICmdList, fillGridList, fillGridListPassParameters,
			FIntVector(FMath::DivideAndRoundUp(50, NUM_THREADS_PER_GROUP_DIMENSION),
				FMath::DivideAndRoundUp(50, NUM_THREADS_PER_GROUP_DIMENSION), 20));

		//*******************************************************************
		//                fill particle ID              ************************
		//*******************************************************************
		FillParticlieID::FParameters fillParticlieIDPassParameters;
		fillParticlieIDPassParameters.grid_x = grid_x;
		fillParticlieIDPassParameters.grid_y = grid_y;
		fillParticlieIDPassParameters.grid_z = grid_z;
		fillParticlieIDPassParameters.listCur = listCurBufferUAV;
		fillParticlieIDPassParameters.particle_id = particle_idBufferUAV;
		fillParticlieIDPassParameters.Position = PositionBufferUAV;
		fillParticlieIDPassParameters.grid_size = grid_size;
		fillParticlieIDPassParameters.numOfParticle = numOfCubes;

		TShaderMapRef<FillParticlieID> fillParticlieID(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		RHICmdList.BlockUntilGPUIdle();
		FComputeShaderUtils::Dispatch(RHICmdList, fillParticlieID, fillParticlieIDPassParameters,
			FIntVector(FMath::DivideAndRoundUp(32, NUM_THREADS_PER_GROUP_DIMENSION),
				FMath::DivideAndRoundUp(32, NUM_THREADS_PER_GROUP_DIMENSION), 1));


	

		//*******************************************************************
		//                collision                  ************************
		//*******************************************************************

		Collision::FParameters collisionPassParameters;

		collisionPassParameters.Position = PositionBufferUAV;
		collisionPassParameters.Velocity = VelocityBufferUAV;
		collisionPassParameters.Force = ForceBufferUAV;
		collisionPassParameters.listHead = listHeadBufferUAV;
		collisionPassParameters.listTail = listTailBufferUAV;
		collisionPassParameters.particle_id = particle_idBufferUAV;

		collisionPassParameters.restitution_coef = restitution_coef;
		collisionPassParameters.dt = dt;
		collisionPassParameters.m = m;
		collisionPassParameters.r = r;
		collisionPassParameters.stiffness = stiffness;
		collisionPassParameters.grid_x = grid_x;
		collisionPassParameters.grid_y = grid_y;
		collisionPassParameters.grid_z = grid_z;
		collisionPassParameters.grid_size = grid_size;
		collisionPassParameters.numOfParticle = numOfCubes;
		//Get a reference to our shader type from global shader map


		TShaderMapRef<Collision> collision(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		RHICmdList.BlockUntilGPUIdle();
		FComputeShaderUtils::Dispatch(RHICmdList, collision, collisionPassParameters,
			FIntVector(FMath::DivideAndRoundUp(32, NUM_THREADS_PER_GROUP_DIMENSION),
				FMath::DivideAndRoundUp(32, NUM_THREADS_PER_GROUP_DIMENSION), 1));





	



		
	}



	//copy back to cpu
	FVector4f* srcptr = (FVector4f*)RHILockStructuredBuffer(PositionBuffer, 0, numOfCubes * sizeof(FVector4f), EResourceLockMode::RLM_ReadOnly);

	FMemory::Memcpy(outputPositions.GetData(), srcptr, numOfCubes * sizeof(FVector4f));
	RHIUnlockStructuredBuffer(PositionBuffer.GetReference());

	//for (int i = 0; i < numOfCubes; i++) {
	//	UE_LOG(LogTemp, Warning, TEXT("{ i : %i,position : {%f,%f,%f,%f}"), i,outputPositions[i].X, outputPositions[i].Y, outputPositions[i].Z, outputPositions[i].W);
	//}

	
	/*
	
	//test buffer 
	//copy back to cpu
	int* srcptr_1 = (int*)RHILockStructuredBuffer(particle_idBuffer, 0, numOfCubes * sizeof(int), EResourceLockMode::RLM_ReadOnly);
	grid.Init(3, numOfCubes);
	FMemory::Memcpy(grid.GetData(), srcptr_1, numOfCubes * sizeof(int) );
	RHIUnlockStructuredBuffer(particle_idBuffer.GetReference());

	UE_LOG(LogTemp, Warning, TEXT("{position : {%i,%i}"), 4, grid[4]);

	*/

	


	
	
	//test buffer
	//copy back to cpu
//	int* srcptr_1 = (int*)RHILockStructuredBuffer(listTailBuffer, 0, grid_x * grid_y  *grid_z* sizeof(int), EResourceLockMode::RLM_ReadOnly);
//	grid.Init(3, grid_x * grid_y * grid_z );
//	FMemory::Memcpy(grid.GetData(), srcptr_1, grid_x * grid_y  * grid_z * sizeof(int) );
//	RHIUnlockStructuredBuffer(listTailBuffer.GetReference());

	//UE_LOG(LogTemp, Warning, TEXT("{position : {%i,%i}"), 1275, grid[1275]);
	//UE_LOG(LogTemp, Warning, TEXT("{position : {%i,%i}"), 1274, grid[1274]);
	
	
	


}