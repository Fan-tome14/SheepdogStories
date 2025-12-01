// Fill out your copyright notice in the Description page of Project Settings.


#include "SheepSpawner.h"
#include "PawnSheep.h"

// Sets default values
ASheepSpawner::ASheepSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASheepSpawner::BeginPlay()
{
	Super::BeginPlay();
	for (int i = 0; i < NumberOfSheep; i++)
	{
		FVector SpawnLocation = GetActorLocation() + GetRandomVectorInRadius(SpawnRadius);
		FRotator SpawnRotation(0, FMath::RandRange(-180, 180), 0);
		GetWorld()->SpawnActor<APawnSheep>(SheepClass, SpawnLocation, SpawnRotation);
	}
	
}

// Called every frame
void ASheepSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ASheepSpawner::GetRandomVectorInRadius(float Radius)
{
	FVector randomVector;
	randomVector.X = FMath::FRandRange(-Radius, Radius);
	randomVector.Y = FMath::FRandRange(-Radius, Radius);
	randomVector.Z = 0;
	return randomVector;
}

