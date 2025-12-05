// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Sheep.generated.h"

class AMyCharacter;

UCLASS()
class SHEEPDOGSTORIES_API ASheep : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASheep();

	//Fonction de séparation 
	FVector CalculateSeparationForce(const TArray<ASheep*>& Neighbors, float SeparationRadius) const;
	FVector CalculateCohesionForce(const TArray<ASheep*> & Neighbors) const;
	FVector CalculateAlignmentForce(const TArray<ASheep*>& Neighbors) const;
	FVector CalculateDogRepulsionForce(const FVector& DogLocation, float RepulsionRadius) const;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Composant de collusion pour détection des voisins et du chien
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionBox;

	//Ref au chien
	UPROPERTY()
	AMyCharacter* Dogref;
	
	static constexpr float SEPARATION_RADIUS = 200.0f; // Rayon pour détecter les voisins trop proches
	static constexpr float SEPARATION_WEIGHT = 5.0f;  // Poids de l'influence de la séparation
	static constexpr float MAX_SPEED = 300.0f;

	static constexpr float STOP_THRESHOLD_SQUARED = 0.05f * 0.05f;

	static constexpr float COHESION_WEIGHT = 2.0f;

	static constexpr float ALIGNMENT_WEIGHT = 0.1f;

	static constexpr float DOG_REPULSION_RADIUS = 600.0f; 
	static constexpr float DOG_FEAR_WEIGHT = 10.0f;

	static constexpr float GRAVITY_Z = -980.0f;

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* SheepMesh;*/
};
