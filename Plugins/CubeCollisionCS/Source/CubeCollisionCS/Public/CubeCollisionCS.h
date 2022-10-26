// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

#include "RenderGraphResources.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"


//class  CUBECOLLISIONCS_API FCubeCollisionCSModule : public IModuleInterface


struct CubeCollisionCSParameters
{
	UTextureRenderTarget2D* VelocityRenderTarget;
	UTextureRenderTarget2D* PositionRenderTarget;
	UTextureRenderTarget2D* ForceRenderTarget;
	UTextureRenderTarget2D* ACCRenderTarget;
	float m;
	float dt;

	//***************************construct Functions ********************************************************
	CubeCollisionCSParameters() {

	}

	FIntPoint GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}

private:
	FIntPoint CachedRenderTargetSize;
};

class  CUBECOLLISIONCS_API  FCubeCollision : public IModuleInterface
{
public:

	static inline FCubeCollision& Get()
	{
		return FModuleManager::LoadModuleChecked<FCubeCollision>("CubeCollisionCS");
	}


	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:
	void BeginRendering(int numofActor = 1024);
	void EndRendering();

	//output positions
	TArray<FVector4f> outputPositions;


	//test buffer
	TArray<int> grid;

private:

	//The delegate handle to our function that will be executed each frame by the renderer
	FDelegateHandle OnPostResolvedSceneColorHandle;

	// make sure that params are updated
	volatile bool bCachedParametersValid;

	//Cached Shader Manager Parameters
	CubeCollisionCSParameters cachedParams;


	//GPU side;
	//FBufferRHIRef PositionBuffer;
	FStructuredBufferRHIRef PositionBuffer;
	FUnorderedAccessViewRHIRef PositionBufferUAV;     // we need a UAV for writing

	FStructuredBufferRHIRef VelocityBuffer;
	FUnorderedAccessViewRHIRef VelocityBufferUAV;     // we need a UAV for writing

	FStructuredBufferRHIRef ForceBuffer;
	FUnorderedAccessViewRHIRef ForceBufferUAV;     // we need a UAV for writing

	FStructuredBufferRHIRef AccBuffer;
	FUnorderedAccessViewRHIRef AccBufferUAV;     // we need a UAV for writing

	FStructuredBufferRHIRef particleNumInGridBuffer;
	FUnorderedAccessViewRHIRef particleNumInGridBufferUAV;     // we need a UAV for writing

	FStructuredBufferRHIRef zSumBuffer;
	FUnorderedAccessViewRHIRef zSumBufferUAV;     // we need a UAV for writing


	FStructuredBufferRHIRef prefixSumBuffer;
	FUnorderedAccessViewRHIRef prefixSumBufferUAV;     // we need a UAV for writing

	FStructuredBufferRHIRef listHeadBuffer;
	FUnorderedAccessViewRHIRef listHeadBufferUAV;     // we need a UAV for writing

	FStructuredBufferRHIRef listCurBuffer;
	FUnorderedAccessViewRHIRef listCurBufferUAV;     // we need a UAV for writing

	FStructuredBufferRHIRef listTailBuffer;
	FUnorderedAccessViewRHIRef listTailBufferUAV;     // we need a UAV for writing

	FStructuredBufferRHIRef particle_idBuffer;
	FUnorderedAccessViewRHIRef particle_idBufferUAV;     // we need a UAV for writing


	//
	int numOfCubes;
	float xb = 250;
	float yb = 250;
	float zb = 100;

	int grid_size = 10;
	int grid_x = 50;
	int grid_y = 50;
	int grid_z = 20;

	float r = 3;
	float stiffness = 1e4;
	float m = 5;
	float restitution_coef = 0.001;
	float dt = 0.0002;




	void Execute_updateKernel(FRHICommandListImmediate& RHICmdList);
	void Execute_BCKernel(FRHICommandListImmediate& RHICmdList);

	void Execute_RenderThread(FRDGBuilder& builder, const FSceneTextures& SceneTextures);

};
