// Copyright Epic Games, Inc. All Rights Reserved.

#include "CubeCollisionCS.h"
#include "Interfaces/IPluginManager.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "RenderGraphBuilder.h"
#include"RHI.h"


#define LOCTEXT_NAMESPACE "FCubeCollision"


void  FCubeCollision::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("CubeCollisionCS"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/CubeCollisionCS"), PluginShaderDir);
}

void  FCubeCollision::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}


void FCubeCollision::BeginRendering(int numofActor) {
	//If the handle is already initalized and valid, no need to do anything
	if (OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}
	//Get the Renderer Module and add our entry to the callbacks so it can be executed each frame after the scene rendering is done

	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &FCubeCollision::Execute_RenderThread);
	}

	//initialize structure buffer
	const size_t size = sizeof(FVector4f);

	TResourceArray<FVector4f> resourceArray;
	TResourceArray<FVector4f> VelocityArray;

	TResourceArray<int> particleNumInGridArray;
	TResourceArray<int> zSumArray;
	TResourceArray<int> prefixSumArray;
	TResourceArray<int> particleIDArray;

	const FVector4f zero(0.0f,0.0f,0.0f,0.0f);
	const FVector4f vel(0.0f, 0.0f, 0.0f, 0.0f);

	//resourceArray.Init(zero, numofActor);
	for (int i = 0; i < numofActor; i++) {
		
		resourceArray.Add(zero);
		VelocityArray.Add(vel);
		
	}

	FRHIResourceCreateInfo createInfo(TEXT("position_array"));
	createInfo.ResourceArray = &resourceArray;

	ForceBuffer = RHICreateStructuredBuffer(size, size * numofActor, BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
	ForceBufferUAV = RHICreateUnorderedAccessView(ForceBuffer, false, false);

	AccBuffer = RHICreateStructuredBuffer(size, size * numofActor, BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
	AccBufferUAV = RHICreateUnorderedAccessView(AccBuffer, false, false);


	particleNumInGridArray.Init(0, grid_x * grid_y * grid_z);
	zSumArray.Init(0, grid_x * grid_y);
	prefixSumArray.Init(0, grid_x * grid_y);
	particleIDArray.Init(0, numofActor);

	for (int i = 0; i < numofActor; i++) {
		float posX = FMath::RandRange(-200.0f, 200.f);
		float posY = FMath::RandRange(-200.0f, 200.f);
		float posZ = FMath::RandRange( 0.0f, 100.f);

		//posX = 0;
		posY = 0;


		resourceArray[i] = FVector4f(posX, posY, posZ, 0.0f);
		int floor_x = floor((posX + xb) / grid_size);
		int floor_y = floor((posY + yb) / grid_size);
		int floor_z = floor((posZ + zb) / grid_size);

		particleNumInGridArray[floor_z * grid_x * grid_y + floor_y * grid_x + floor_x] += 1;

		
	}

	PositionBuffer = RHICreateStructuredBuffer(size, size * numofActor, BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
	PositionBufferUAV = RHICreateUnorderedAccessView(PositionBuffer, false, false);

	createInfo.ResourceArray = &VelocityArray;
	VelocityBuffer = RHICreateStructuredBuffer(size, size * numofActor, BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
	VelocityBufferUAV = RHICreateUnorderedAccessView(VelocityBuffer, false, false);

	createInfo.ResourceArray = &particleNumInGridArray;
	particleNumInGridBuffer = RHICreateStructuredBuffer(sizeof(int), sizeof(int) * grid_x * grid_y * grid_z, BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
	particleNumInGridBufferUAV = RHICreateUnorderedAccessView(particleNumInGridBuffer, false, false);

	listHeadBuffer = RHICreateStructuredBuffer(sizeof(int), sizeof(int) * grid_x * grid_y * grid_z, BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
	listHeadBufferUAV = RHICreateUnorderedAccessView(listHeadBuffer, false, false);

	listCurBuffer = RHICreateStructuredBuffer(sizeof(int), sizeof(int) * grid_x * grid_y * grid_z, BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
	listCurBufferUAV = RHICreateUnorderedAccessView(listCurBuffer, false, false);


	listTailBuffer = RHICreateStructuredBuffer(sizeof(int), sizeof(int) * grid_x * grid_y * grid_z, BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
	listTailBufferUAV = RHICreateUnorderedAccessView(listTailBuffer, false, false);



	createInfo.ResourceArray = &zSumArray;
	zSumBuffer = RHICreateStructuredBuffer(sizeof(int), sizeof(int) * grid_x * grid_y , BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
	zSumBufferUAV = RHICreateUnorderedAccessView(zSumBuffer, false, false);

	createInfo.ResourceArray = &prefixSumArray;
	prefixSumBuffer = RHICreateStructuredBuffer(sizeof(int), sizeof(int) * grid_x * grid_y, BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
	prefixSumBufferUAV = RHICreateUnorderedAccessView(prefixSumBuffer, false, false);

	
	createInfo.ResourceArray = &particleIDArray;
	particle_idBuffer = RHICreateStructuredBuffer(sizeof(int), sizeof(int) * numofActor, BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
	particle_idBufferUAV = RHICreateUnorderedAccessView(particle_idBuffer, false, false);



	outputPositions.Init(zero, numofActor);

	numOfCubes = numofActor;
}


//Stop the compute shader execution
void FCubeCollision::EndRendering()
{
	//If the handle is not valid then there's no cleanup to do
	if (!OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}
	//Get the Renderer Module and remove our entry from the ResolvedSceneColorCallbacks
	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}

	OnPostResolvedSceneColorHandle.Reset();

}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCubeCollision, CubeCollisionCS)