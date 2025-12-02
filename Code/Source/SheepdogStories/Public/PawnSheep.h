// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "PawnSheep.generated.h"

UCLASS()
class SHEEPDOGSTORIES_API APawnSheep : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APawnSheep();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USphereComponent* LocalFlockArea;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UBoxComponent* CollisionBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components");
	class USkeletalMeshComponent* SheepMesh;
	
	// Movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement");
	float MinSpeed = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement");
	float MaxSpeed = 300.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement");
	FVector Velocity;
	
	// Flocking weights
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flocking");
	float m_fWeightAlignment = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flocking");
	float m_fWeightCohesion = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flocking");
	float m_fWeightSeparation = 2.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flocking");
	float SeparationRadius = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flocking");
	float AlignmentRadius = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flocking");
	float CohesionRadius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flocking");
	float SteeringStrength = 200.f;

	// Flocking behaviors
	FVector Flocking();
	FVector Alignment(TArray<AActor*> LocalFlock);
	FVector Cohesion(TArray<AActor*> LocalFlock);
	FVector Separation(TArray<AActor*> LocalFlock);

};
