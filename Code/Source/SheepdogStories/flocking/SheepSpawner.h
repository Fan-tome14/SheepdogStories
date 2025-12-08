// SheepSpawner.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SheepSpawner.generated.h"

class UBoxComponent;
class ASheep; // Forward declaration

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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "spawning")
	UBoxComponent* SpawnVolume;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Sheep")
	TSubclassOf<ASheep> SheepClass;

	UPROPERTY(EditAnywhere, Category = "Sheep", meta = (ClampMin = "1", ClampMax = "500"))
	int32 NumberOfSheepToSpawn = 20;

	void SpawnSheep();
};