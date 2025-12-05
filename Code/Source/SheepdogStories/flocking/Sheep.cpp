// Fill out your copyright notice in the Description page of Project Settings.


#include "Sheep.h"

#include "SQCapture.h"


// Sets default values
ASheep::ASheep()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Box de collision
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));

	//Profil de collision pour l'overlap
	CollisionBox->SetCollisionProfileName(TEXT("Pawn"));
	
	//Static Mesh 
	/*SheepMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SheepMesh"));
	SheepMesh->SetupAttachment(CollisionBox);*/
}

FVector ASheep::CalculateSeparationForce(const TArray<ASheep*>& Neighbors, float SeparationRadius) const
{
	FVector SeparationForce = FVector::ZeroVector;
	int32 Count = 0;

	//Pour un mouton
	if (Neighbors.Num() == 0)
	{
		return SeparationForce;
	}

	// 1.Parcours des moutons voisins
	for (ASheep* Neighbor : Neighbors)
	{
		// On s'assure de ne pas se comparer a soit meme
		if (Neighbor == this)
		{
			continue;
		}

		//2. Calcul de la distance entre le mouton actuel et son voison
		FVector Direction = GetActorLocation() - Neighbor->GetActorLocation();
		float Distance = Direction.Size();

		//3. Appliquer la force dis la distance est inférieure au rayon de séparation
		if (Distance < SeparationRadius && Distance > 0.0f)
		{
			//Plus le mouton est proche, plus la force de répulsion est forte
			float Strength = 1.0f - (Distance / SeparationRadius);

			SeparationForce += Direction.GetSafeNormal() * Strength;
			Count++;
		}
	}

	//4. Si on a des voisins
	if (Count > 0)
	{
		SeparationForce /= (float)Count;
		SeparationForce = SeparationForce.GetClampedToMaxSize(1.0f); 
	}
	
	return SeparationForce;
}

FVector ASheep::CalculateCohesionForce(const TArray<ASheep*>& Neighbors) const
{
	// Le cas ou y'a pas de voisin
	if (Neighbors.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	FVector CenterOfMass = FVector::ZeroVector;

	// Calculer le centre de masse
	for (ASheep* Neighbor : Neighbors)
	{
		CenterOfMass += Neighbor->GetActorLocation();
	}

	//Calculer la position moyenne du centre de masse
	CenterOfMass /= (float)Neighbors.Num();

	// Création du vecteur de force pointant vers le centre de masse
	FVector CohesionForce = CenterOfMass - GetActorLocation();

	// On normalise la force
	return CohesionForce.GetSafeNormal();
}

FVector ASheep::CalculateAlignmentForce(const TArray<ASheep*>& Neighbors) const
{
	
	if (Neighbors.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	FVector AverageVelocity = FVector::ZeroVector;

	// 2. Calculer la somme des vélocités des voisins
	for (ASheep* Neighbor : Neighbors)
	{
		AverageVelocity += Neighbor->GetActorForwardVector();
	}

	// 3. Calculer la vélocité moyenne
	AverageVelocity /= (float)Neighbors.Num();
	
	return AverageVelocity.GetSafeNormal();
}

FVector ASheep::CalculateDogRepulsionForce(const FVector& DogLocation, float RepulsionRadius) const
{
	FVector RepulsionForce = FVector::ZeroVector;

	// 1. Calculer la distance et la direction du chien
	FVector DirectionToDog = DogLocation - GetActorLocation();
	float Distance = DirectionToDog.Size();

	// 2. Appliquer la force uniquement si le chien est dans le rayon de peur
	if (Distance < RepulsionRadius && Distance > 0.0f)
	{
		// Direction de fuite : loin du chien
		FVector DirectionOfFlight = DirectionToDog.GetSafeNormal() * -1.0f;

		//Intensité
		float Strength = 1.0f - (Distance / RepulsionRadius); 
        
		RepulsionForce = DirectionOfFlight * Strength;
	}
	
	return RepulsionForce.GetClampedToMaxSize(1.0f);
}

// Called when the game starts or when spawned
void ASheep::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASheep::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. Trouver l'emplacement du chien (le joueur)
	FVector DogLocation = FVector::ZeroVector;
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    
	if (PlayerPawn)
	{
		DogLocation = PlayerPawn->GetActorLocation();
	}

	//1. Trouver les voisins
	TArray<AActor*> OverlappingActors;
	CollisionBox->GetOverlappingActors(OverlappingActors, ASheep::StaticClass());

	//Convertion en TArray de ASheepPawn*
	TArray<ASheep*> Neighbors;
	for (AActor* Actor : OverlappingActors)
	{
		ASheep* NeighborSheep = Cast<ASheep>(Actor);
		if (NeighborSheep && NeighborSheep != this)
		{
			Neighbors.Add(NeighborSheep);
		}
	}

	//2. Calculer les forces

	
	
	FVector TotalForce = FVector::ZeroVector;

	// A. Répulsion du Chien 
	FVector DogRepulsionForce = CalculateDogRepulsionForce(DogLocation, DOG_REPULSION_RADIUS);
	TotalForce += DogRepulsionForce * DOG_FEAR_WEIGHT;
	

	//Calcul de la séparation
	FVector SeparationForce = CalculateSeparationForce(Neighbors, SEPARATION_RADIUS);
	TotalForce += SeparationForce * SEPARATION_WEIGHT;

	// Cohesion
	FVector CohesionForce = CalculateCohesionForce(Neighbors);
	TotalForce += CohesionForce * COHESION_WEIGHT;

	//Alignement
	FVector AlignmentForce = CalculateAlignmentForce(Neighbors);
	TotalForce += AlignmentForce * ALIGNMENT_WEIGHT;

	
	

	// on s'assure que la forece totale ne dépasse pas une magnitude raisonnable
	if (!TotalForce.IsNearlyZero())
	{
		//on normalize la force et on l'applique sur la vitesse maximale
		FVector DesiredVelocity = TotalForce.GetSafeNormal() * MAX_SPEED;

		DesiredVelocity.Z += GRAVITY_Z * DeltaTime;

		//Mouvement simple
		FVector NewLocation = GetActorLocation() + DesiredVelocity * DeltaTime;

		if (NewLocation.Z < 50.0f) // 50.0f est la moitié de la Box Extent
		{
			NewLocation.Z = 10.0f;
		}
		
		SetActorLocation(NewLocation, true);

		//Rotation : faire regarder le mouton dans la direction de son mouvement
		FRotator NewRotation = DesiredVelocity.Rotation();
		SetActorRotation(NewRotation);
	}

	
}

// Called to bind functionality to input
void ASheep::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

