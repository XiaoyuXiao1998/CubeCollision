// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CubeActor.generated.h"

UCLASS()
class CUBECOLLISION_API ACubeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACubeActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CubeCollisionCS)
		float dt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CubeCollisionCS)
		float m;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CubeCollisionCS)
		int numofActor;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CubeCollisionCS)
		TArray<AActor*> actorList;





protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
