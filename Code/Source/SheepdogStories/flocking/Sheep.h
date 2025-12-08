// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sheep.generated.h"


UCLASS()
class SHEEPDOGSTORIES_API ASheep : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASheep();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//Fonction de séparation 
	FVector CalculateSeparationForce(const TArray<ASheep*>& Neighbors, float SeparationRadius) const;
	FVector CalculateCohesionForce(const TArray<ASheep*> & Neighbors) const;
	FVector CalculateAlignmentForce(const TArray<ASheep*>& Neighbors) const;
	FVector CalculateDogRepulsionForce(const FVector& DogLocation, float RepulsionRadius) const;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Composant de collusion pour détection des voisins et du chien
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCapsuleComponent* CapsuleCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* SheepMesh;

	
	static constexpr float MAX_SPEED = 400.0f;

	static constexpr float SEPARATION_WEIGHT = 2.5f;  
	static constexpr float COHESION_WEIGHT = 0.8f;
	static constexpr float ALIGNMENT_WEIGHT = 1.2f;
	
	static constexpr float DOG_FEAR_WEIGHT = 10.0f;

	static constexpr float SEPARATION_RADIUS = 60.0f; 
	static constexpr float DOG_REPULSION_RADIUS = 800.0f; 

};
