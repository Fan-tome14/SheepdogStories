// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnSheep.h"

#include "Components/BoxComponent.h"

// Sets default values
APawnSheep::APawnSheep()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	SheepMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SheepMesh"));
	SheepMesh->SetupAttachment(RootComponent);

	LocalFlockArea = CreateDefaultSubobject<USphereComponent>(TEXT("LocalFlockArea"));
	LocalFlockArea->SetupAttachment(RootComponent);
	LocalFlockArea->SetSphereRadius(500.f);
	LocalFlockArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LocalFlockArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	LocalFlockArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}

// Called when the game starts or when spawned
void APawnSheep::BeginPlay()
{
	Super::BeginPlay();
	Velocity = GetActorForwardVector();
	Velocity = Velocity.GetSafeNormal();
	Velocity *= FMath::RandRange(MinSpeed, MaxSpeed);
}

// Called every frame
void APawnSheep::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Acceleration =  FVector(0, 0, 0);
	Velocity += Acceleration* DeltaTime;
	Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);
	FVector NewLocation = GetActorLocation() + (Velocity * DeltaTime);
	SetActorLocation(NewLocation);

}

FVector APawnSheep::Flocking()
{
	FVector steeringForce;
	FVector alignment = Alignment() * m_fWeightAlignment;
	FVector cohesion = Cohesion() * m_fWeightCohesion;
	FVector separation = Separation() * m_fWeightSeparation;
	steeringForce += separation;
	steeringForce += alignment;
	steeringForce += cohesion;
	return steeringForce;
}

FVector APawnSheep::Alignment()
{
	FVector alignment;
	return alignment;
}

FVector APawnSheep::Cohesion()
{
	FVector cohesion;
	return cohesion;
}

FVector APawnSheep::Separation()
{
	FVector separation;
	return separation;
}