// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SheepSpawner.generated.h"

UCLASS()
class SHEEPDOGSTORIES_API ASheepSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASheepSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class APawnSheep> SheepClass;

	UPROPERTY(EditAnywhere)
	int NumberOfSheep = 30;

	UPROPERTY(EditAnywhere)
	int SpawnRadius = 500;

protected:
	FVector GetRandomVectorInRadius(float Radius);
	
};
