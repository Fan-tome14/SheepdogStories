// SheepSpawner.cpp

#include "SheepSpawner.h"
#include "Sheep.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

// Sets default values
ASheepSpawner::ASheepSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    SetRootComponent(SpawnVolume);

    SpawnVolume->SetBoxExtent(FVector(1000.f, 1000.f, 10.f));
}

void ASheepSpawner::BeginPlay()
{
    Super::BeginPlay();
    SpawnSheep();
}

/* -------------------------------------------------------
      Vérifie si un endroit est libre avant de spawn
------------------------------------------------------- */
bool ASheepSpawner::IsSpawnLocationFree(const FVector& Location, float Radius) const
{
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);

    // On cherche des acteurs de type Pawn (les moutons)
    bool bHit = GetWorld()->OverlapAnyTestByObjectType(
        Location,
        FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn),
        Sphere
    );

    return !bHit; // true si libre
}

void ASheepSpawner::SpawnSheep()
{
    if (!SheepClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SheepSpawner : aucune classe de mouton assignée !"));
        return;
    }

    FVector Origin = SpawnVolume->GetComponentLocation();
    FVector Extent = SpawnVolume->GetScaledBoxExtent();

    for (int32 i = 0; i < NumberOfSheepToSpawn; i++)
    {
        FVector SpawnLocation;
        int32 Tries = 0;

        // On cherche un emplacement libre (max 20 essais)
        do
        {
            SpawnLocation = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);
            SpawnLocation.Z = 10.f;

            Tries++;
        }
        while (!IsSpawnLocationFree(SpawnLocation, 80.f) && Tries < 20);

        // Gestion du spawn
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

        GetWorld()->SpawnActor<ASheep>(SheepClass, SpawnLocation, FRotator::ZeroRotator, Params);
    }
}
