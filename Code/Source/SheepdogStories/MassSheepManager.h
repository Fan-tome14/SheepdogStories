#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassArchetypeTypes.h"
#include "MassEntityHandle.h"
#include "MassSheepManager.generated.h"

class UMassEntitySubsystem;
class UInstancedStaticMeshComponent;

/**
 * Manager qui spawn et gère les moutons avec Mass Entity
 */
UCLASS()
class SHEEPDOGSTORIES_API AMassSheepManager : public AActor
{
    GENERATED_BODY()

public:
    AMassSheepManager();

    // Nombre de moutons à spawner
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 NumberOfSheep = 50;

    // Rayon de spawn autour du manager
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnRadius = 2000.f;

    // Mesh du mouton
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
    TObjectPtr<UStaticMesh> SheepMesh;

    // Couleur des moutons (optionnel)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
    FLinearColor SheepColor = FLinearColor::White;

    // Composant pour afficher tous les moutons
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visualization")
    TObjectPtr<UInstancedStaticMeshComponent> SheepInstances;

    // Paramètres IA - Comportement de troupeau
    UPROPERTY(EditAnywhere, Category = "Sheep AI | Flock Behavior")
    float CohesionRadius = 300.f;  // Distance pour rester groupés

    UPROPERTY(EditAnywhere, Category = "Sheep AI | Flock Behavior")
    float SeparationRadius = 80.f;  // Distance minimale entre moutons

    UPROPERTY(EditAnywhere, Category = "Sheep AI | Flock Behavior")
    float AlignmentRadius = 250.f;  // Distance pour s'aligner

    UPROPERTY(EditAnywhere, Category = "Sheep AI | Dog Response")
    float PanicDistance = 150.f;  // Distance où ils commencent à fuir (RÉDUIT)

    UPROPERTY(EditAnywhere, Category = "Sheep AI | Dog Response")
    float AlertDistance = 400.f;  // Distance où ils deviennent nerveux (RÉDUIT)

    UPROPERTY(EditAnywhere, Category = "Sheep AI | Speeds")
    float GrazingSpeed = 25.f;

    UPROPERTY(EditAnywhere, Category = "Sheep AI | Speeds")
    float WalkSpeed = 100.f;

    UPROPERTY(EditAnywhere, Category = "Sheep AI | Speeds")
    float RunSpeed = 300.f;

    // Offset vertical pour compenser le pivot du mesh (ajuste selon ton mesh)
    UPROPERTY(EditAnywhere, Category = "Visualization")
    float MeshHeightOffset = 50.f;

    // Offset de rotation si le mesh n'est pas orienté vers l'avant (en degrés)
    // 0 = correct, -90 = mesh pointe à droite, 90 = mesh pointe à gauche, 180 = mesh à l'envers
    UPROPERTY(EditAnywhere, Category = "Visualization")
    float MeshRotationOffset = -90.f;

    // === VARIÉTÉ DES MOUTONS ===
    
    // Activer la variété de taille
    UPROPERTY(EditAnywhere, Category = "Visualization | Variety")
    bool bEnableSizeVariety = true;

    // Taille minimale (1.0 = taille normale)
    UPROPERTY(EditAnywhere, Category = "Visualization | Variety", meta = (EditCondition = "bEnableSizeVariety"))
    float MinScale = 0.6f;

    // Taille maximale (1.0 = taille normale)
    UPROPERTY(EditAnywhere, Category = "Visualization | Variety", meta = (EditCondition = "bEnableSizeVariety"))
    float MaxScale = 1.4f;

    // === ANTI-BLOCAGE ===
    
    // Vitesse minimale considérée comme "mouvement" (détection blocage)
    UPROPERTY(EditAnywhere, Category = "Sheep AI | Anti-Blocking")
    float MinMovementSpeed = 10.f;

    // Multiplicateur de force quand coincé
    UPROPERTY(EditAnywhere, Category = "Sheep AI | Anti-Blocking")
    float BlockedEscapeMultiplier = 3.0f;

    // Lissage de la direction (0 = pas de lissage, 1 = très lisse)
    UPROPERTY(EditAnywhere, Category = "Sheep AI | Anti-Blocking")
    float DirectionSmoothness = 0.3f;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void SpawnAllSheep();
    void UpdateSheepAI(float DeltaTime);
    void UpdateVisualization();

    TArray<FMassEntityHandle> SheepEntities;
    UMassEntitySubsystem* MassEntitySubsystem;
};
