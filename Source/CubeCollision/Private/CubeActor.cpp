// Fill out your copyright notice in the Description page of Project Settings.

#include "CubeCollisionCS.h"
#include "CubeActor.h"

// Sets default values
ACubeActor::ACubeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m = 100;
	dt = 0.0005;
	numofActor = 100;
}

// Called when the game starts or when spawned
void ACubeActor::BeginPlay()
{
	Super::BeginPlay();

	//invoke compute shader;
	FCubeCollision::Get().BeginRendering(numofActor);

	
}


void  ACubeActor::BeginDestroy()
{
	FCubeCollision::Get().EndRendering();
	Super::BeginDestroy();
}

// Called every frame
void ACubeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	
	TArray<FVector4f>& outputPositions = FCubeCollision::Get().outputPositions;
	//UE_LOG(LogTemp, Warning, TEXT("{force : {%f,%f,%f,%f}"), outputPositions[0].X, outputPositions[0].Y, outputPositions[0].Z, outputPositions[0].W);

	for (int i = 0; i < actorList.Num()&& i < numofActor;i++)
	{
		AActor* p = actorList[i];

		//set location
	    p->SetActorLocation(FVector(float(outputPositions[i].X), float(outputPositions[i].Y), float(outputPositions[i].Z)));


	}




}

