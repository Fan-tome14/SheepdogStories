// SheepSpawner.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SheepSpawner.generated.h"

class UBoxComponent;
class ASheep;

UCLASS()
class SHEEPDOGSTORIES_API ASheepSpawner : public AActor
{
	GENERATED_BODY()

public:
	ASheepSpawner();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawning")
	UBoxComponent* SpawnVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sheep")
	TSubclassOf<ASheep> SheepClass;

	UPROPERTY(EditAnywhere,  BlueprintReadWrite, Category = "Sheep", meta = (ClampMin = "1", ClampMax = "500"))
	int32 NumberOfSheepToSpawn = 20;

	UFUNCTION()
	void SpawnSheep();

private:

	/** Vérifie qu'une zone autour de Location n'est pas déjà occupée */
	bool IsSpawnLocationFree(const FVector& Location, float Radius) const;
};
