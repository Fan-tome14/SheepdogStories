#include "SheepSpawner.h"
#include "Sheep.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h" 

// Sets default values
ASheepSpawner::ASheepSpawner()
{
    PrimaryActorTick.bCanEverTick = false; 

    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    SetRootComponent(SpawnVolume);
    
    SpawnVolume->SetBoxExtent(FVector(1000.0f, 1000.0f, 10.0f));
}

void ASheepSpawner::BeginPlay()
{
    Super::BeginPlay();

    SpawnSheep();
}

void ASheepSpawner::SpawnSheep()
{
    if (!SheepClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SheepSpawner: Aucune classe de mouton définie !"));
        return;
    }

    if (!SpawnVolume) return;

    for (int32 i = 0; i < NumberOfSheepToSpawn; i++)
    {
        FVector Origin = SpawnVolume->GetComponentLocation();
        FVector BoxExtent = SpawnVolume->GetScaledBoxExtent();

        FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, BoxExtent);

        FRotator Rotation = FRotator::ZeroRotator;
        Rotation.Yaw = FMath::RandRange(0.0f, 360.0f); 

        // 3. Spawner l'acteur
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        GetWorld()->SpawnActor<ASheep>(SheepClass, RandomPoint, Rotation, SpawnParams);
    }
}