// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnSheep.h"

#include "Components/BoxComponent.h"

// Sets default values
APawnSheep::APawnSheep()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	//CollisionBox->SetupAttachment(RootComponent);
	RootComponent = CollisionBox;
	
	SheepMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SheepMesh"));
	SheepMesh->SetupAttachment(RootComponent);

	LocalFlockArea = CreateDefaultSubobject<USphereComponent>(TEXT("LocalFlockArea"));
	LocalFlockArea->SetupAttachment(RootComponent);
	LocalFlockArea->SetSphereRadius(600.f);
	LocalFlockArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LocalFlockArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	LocalFlockArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}

// Called when the game starts or when spawned
void APawnSheep::BeginPlay()
{
	Super::BeginPlay();
	/*Velocity = GetActorForwardVector();
	Velocity = Velocity.GetSafeNormal();
	Velocity *= FMath::RandRange(MinSpeed, MaxSpeed);*/
}

// Called every frame
void APawnSheep::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//FVector Acceleration =  FVector(0, 0, 0);
	//Velocity += Acceleration* DeltaTime;

	Velocity += Flocking() * SteeringStrength * DeltaTime;
	Velocity = Velocity.GetClampedToSize(MinSpeed, MaxSpeed);
	FVector NewLocation = GetActorLocation() + (Velocity * DeltaTime);
	//SetActorLocation(NewLocation);
	//CollisionBox->SetPhysicsLinearVelocity(Velocity); 
	SetActorRotation(Velocity.ToOrientationQuat());

}

FVector APawnSheep::Flocking()
{
	TArray<AActor*> LocalFlock;
	LocalFlockArea->GetOverlappingActors(LocalFlock, APawnSheep::StaticClass());
	FVector steeringForce = FVector::ZeroVector;
	
	FVector alignment = Alignment(LocalFlock) * m_fWeightAlignment;
	
	FVector cohesion = Cohesion(LocalFlock) * m_fWeightCohesion;
	FVector separation = Separation(LocalFlock) * m_fWeightSeparation;
	
	steeringForce += alignment;
	steeringForce += cohesion;
	steeringForce += separation;
	
	return steeringForce;
}

FVector APawnSheep::Alignment(TArray<AActor*> LocalFlock)
{
	FVector steeringForce = FVector::ZeroVector;

	int FlockCount = 0;

	for (AActor* OtherActor : LocalFlock)
	{
		
		if (OtherActor != nullptr && OtherActor != this)
		{
			APawnSheep* OtherSheep = Cast<APawnSheep>(OtherActor);
			float DistanceToOtherSheep = FVector::Distance(GetActorLocation(), OtherSheep->GetActorLocation());

			if (DistanceToOtherSheep > AlignmentRadius)
			{
				continue;
			}
			steeringForce += OtherSheep->Velocity;

			FlockCount++;
		}
	}
	if (FlockCount > 0)
	{
		steeringForce /= FlockCount;
		steeringForce = steeringForce.GetSafeNormal();
		steeringForce -= Velocity.GetSafeNormal();
	}
	return steeringForce;
}

FVector APawnSheep::Cohesion(TArray<AActor*> LocalFlock)
{
	FVector steeringForce = FVector::ZeroVector;
	FVector AveragePosition = FVector::ZeroVector;

	int FlockCount = 0;

	for (AActor* OtherActor : LocalFlock)
	{
		
		if (OtherActor != nullptr && OtherActor != this)
		{
			float DistanceToOtherSheep = FVector::Distance(GetActorLocation(), OtherActor->GetActorLocation());

			if (DistanceToOtherSheep > AlignmentRadius)
			{
				continue;
			}

			AveragePosition += OtherActor->GetActorLocation();

			FlockCount++;
		}
	}
	if (FlockCount > 0)
	{
		AveragePosition /= FlockCount;
		steeringForce += AveragePosition - GetActorLocation();
		steeringForce = steeringForce.GetSafeNormal();
		steeringForce -= Velocity.GetSafeNormal();
	}
	return steeringForce;
}

FVector APawnSheep::Separation(TArray<AActor*> LocalFlock)
{
	FVector steeringForce = FVector::ZeroVector;
	FVector SeparationDirection = FVector::ZeroVector;
	
	int FlockCount = 0;

	for (AActor* OtherActor : LocalFlock)
	{
		
		if (OtherActor != nullptr && OtherActor != this)
		{
			APawnSheep* OtherSheep = Cast<APawnSheep>(OtherActor);
			float DistanceToOtherSheep = FVector::Distance(GetActorLocation(), OtherSheep->GetActorLocation());

			if (DistanceToOtherSheep > SeparationRadius)
			{
				continue;
			}
			SeparationDirection = GetActorLocation() - OtherSheep->GetActorLocation();
			SeparationDirection = SeparationDirection.GetSafeNormal();
			steeringForce += SeparationDirection;

			FlockCount++;
		}
	}
	if (FlockCount > 0)
	{
		steeringForce /= FlockCount;
		steeringForce = steeringForce.GetSafeNormal();
		steeringForce -= Velocity.GetSafeNormal();
	}
	return steeringForce;
}