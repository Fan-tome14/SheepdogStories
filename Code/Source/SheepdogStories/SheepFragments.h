#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "SheepFragments.generated.h"

// États possibles du mouton
UENUM(BlueprintType)
enum class ESheepState : uint8
{
    Idle,
    Grazing,      // Brouter tranquillement
    Walking,      // Marcher avec le troupeau
    Running       // Fuir le chien
};

// Fragment contenant l'état et les données du mouton
USTRUCT()
struct SHEEPDOGSTORIES_API FSheepStateFragment : public FMassFragment
{
    GENERATED_BODY()

    // État actuel
    UPROPERTY()
    ESheepState CurrentState = ESheepState::Grazing;

    // Distance au chien
    UPROPERTY()
    float DistanceToPlayer = 0.f;

    // Vitesse actuelle
    UPROPERTY()
    float CurrentSpeed = 0.f;

    // Timer pour changer de comportement
    UPROPERTY()
    float StateTimer = 0.f;

    // Direction de déplacement actuelle
    UPROPERTY()
    FVector MovementDirection = FVector::ZeroVector;

    // Centre du troupeau (calculé)
    UPROPERTY()
    FVector FlockCenter = FVector::ZeroVector;

    // Hauteur du sol de spawn
    UPROPERTY()
    float GroundZ = 0.f;

    // Taille unique de ce mouton (pour la variété)
    UPROPERTY()
    float Scale = 1.0f;

    // === ANTI-BLOCAGE ===
    
    // Position précédente pour détecter si coincé
    UPROPERTY()
    FVector PreviousLocation = FVector::ZeroVector;

    // Temps passé bloqué
    UPROPERTY()
    float BlockedTimer = 0.f;

    // Direction précédente pour lisser le mouvement
    UPROPERTY()
    FVector PreviousDirection = FVector::ZeroVector;
};
