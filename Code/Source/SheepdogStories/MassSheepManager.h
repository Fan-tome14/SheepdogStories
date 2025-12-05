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

    // === ROTATION ===

    // Vitesse de rotation normale (0-1, plus élevé = rotation plus rapide)
    UPROPERTY(EditAnywhere, Category = "Sheep AI | Rotation")
    float NormalRotationSpeed = 0.15f;

    // Vitesse de rotation quand grand virage >90° (0-1, plus bas = plus smooth)
    UPROPERTY(EditAnywhere, Category = "Sheep AI | Rotation")
    float SharpTurnRotationSpeed = 0.05f;

    // === DÉTECTION D'OBSTACLES ===

    // Tag requis sur les obstacles pour que les moutons les contournent (ex: "branche")
    UPROPERTY(EditAnywhere, Category = "Sheep AI | Obstacle Avoidance")
    FName ObstacleTagToAvoid = FName("branche");

    // Distance de détection d'obstacles devant le mouton (en cm)
    UPROPERTY(EditAnywhere, Category = "Sheep AI | Obstacle Avoidance")
    float ObstacleDetectionDistance = 150.f;

    // Rayon de détection pour les obstacles (en cm)
    UPROPERTY(EditAnywhere, Category = "Sheep AI | Obstacle Avoidance")
    float ObstacleDetectionRadius = 50.f;

    // Force du virage pour éviter les obstacles (0-1, plus élevé = virage plus serré)
    UPROPERTY(EditAnywhere, Category = "Sheep AI | Obstacle Avoidance")
    float ObstacleAvoidanceStrength = 0.7f;

    // === ZONE DE SÉCURITÉ ===

    // Activer le système de zone de sécurité
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safe Zone")
    bool bEnableSafeZone = true;

    // Référence à l'acteur de la zone (box trigger blueprint)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safe Zone")
    TObjectPtr<AActor> SafeZoneActor;

    // Distance du bord où la force de confinement commence (en cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safe Zone")
    float SafeZoneBorderMargin = 100.f;

    // Force de poussée vers le centre quand près du bord (multiplicateur)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safe Zone")
    float SafeZoneConfinementStrength = 3.0f;

    // Multiplicateur de séparation dans la zone (plus élevé = plus dispersés)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safe Zone")
    float SafeZoneSeparationMultiplier = 3.0f;

    // Afficher le compteur dans la console
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safe Zone | Debug")
    bool bShowDebugLog = true;

    // Intervalle d'affichage périodique en secondes (0 = seulement quand ça change)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safe Zone | Debug")
    float DebugLogInterval = 5.0f;

    // Nombre de moutons actuellement dans la zone (LECTURE SEULE)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Safe Zone")
    int32 SheepInSafeZoneCount = 0;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void SpawnAllSheep();
    void UpdateSheepAI(float DeltaTime);
    void UpdateVisualization();
    void CheckSheepInSafeZone();  // Vérifier quels moutons sont dans la zone

    TArray<FMassEntityHandle> SheepEntities;
    UMassEntitySubsystem* MassEntitySubsystem;

    // Cache pour les bounds de la zone
    FBox SafeZoneBounds;
    bool bSafeZoneBoundsValid = false;

    // Debug : dernier comptage pour détecter les changements
    int32 LastSheepInSafeZoneCount = 0;
    float DebugLogTimer = 0.f;
};