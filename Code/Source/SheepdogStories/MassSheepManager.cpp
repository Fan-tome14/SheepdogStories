#include "MassSheepManager.h"
#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "MassCommonFragments.h"
#include "MassArchetypeTypes.h"
#include "SheepFragments.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/ShapeComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

AMassSheepManager::AMassSheepManager()
{
    PrimaryActorTick.bCanEverTick = true;
    SheepInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SheepInstances"));
    RootComponent = SheepInstances;

    // COLLISION : Box simple sans physique
    SheepInstances->SetCollisionEnabled(ECollisionEnabled::QueryOnly);  // Collision sans physique
    SheepInstances->SetCollisionObjectType(ECC_Pawn);  // Type : Pawn
    SheepInstances->SetCollisionResponseToAllChannels(ECR_Ignore);
    SheepInstances->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);  // Bloquer autres pawns
    SheepInstances->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);  // Bloquer le terrain
    SheepInstances->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);  // Bloquer objets

    // PAS DE PHYSIQUE : Pas de gravité, pas de simulation
    SheepInstances->SetSimulatePhysics(false);
    SheepInstances->SetEnableGravity(false);
    SheepInstances->bApplyImpulseOnDamage = false;

    // VERROUILLER LES ROTATIONS : Toujours droit (CRITIQUE)
    SheepInstances->BodyInstance.bLockXRotation = true;   // Pas de basculement avant/arrière
    SheepInstances->BodyInstance.bLockYRotation = true;   // Pas de basculement gauche/droite
    SheepInstances->BodyInstance.bLockZRotation = false;  // Permet rotation sur Z (direction)
    SheepInstances->BodyInstance.bLockXTranslation = false;
    SheepInstances->BodyInstance.bLockYTranslation = false;
    SheepInstances->BodyInstance.bLockZTranslation = true;  // Verrouiller hauteur
}

void AMassSheepManager::BeginPlay()
{
    Super::BeginPlay();

    MassEntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());

    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassSheepManager: MassEntitySubsystem not found!"));
        return;
    }

    if (SheepMesh)
    {
        SheepInstances->SetStaticMesh(SheepMesh);

        // Réappliquer les paramètres de collision au cas où le mesh les écrase
        SheepInstances->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        SheepInstances->SetSimulatePhysics(false);
        SheepInstances->SetEnableGravity(false);
        SheepInstances->CanCharacterStepUpOn = ECB_No;
    }

    FTimerHandle SpawnTimer;
    GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AMassSheepManager::SpawnAllSheep, 1.0f, false);
}

void AMassSheepManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CheckSheepInSafeZone();  // Vérifier les moutons dans la zone EN PREMIER
    UpdateSheepAI(DeltaTime);
    UpdateVisualization();

    // Incrémenter le timer pour le debug périodique
    if (bShowDebugLog && DebugLogInterval > 0.f)
    {
        DebugLogTimer += DeltaTime;
    }
}

void AMassSheepManager::SpawnAllSheep()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();
    FVector ManagerLocation = GetActorLocation();

    FMassArchetypeHandle Archetype = EntityManager.CreateArchetype(
        TConstArrayView<const UScriptStruct*>({
            FSheepStateFragment::StaticStruct(),
            FTransformFragment::StaticStruct()
            })
    );

    EntityManager.BatchCreateEntities(Archetype, NumberOfSheep, SheepEntities);

    UE_LOG(LogTemp, Log, TEXT("MassSheepManager: Spawned %d sheep entities"), SheepEntities.Num());

    // Spawner en cercle autour du manager pour former un troupeau compact
    float AngleStep = 360.f / NumberOfSheep;
    float CurrentRadius = 100.f;
    int32 SheepInCurrentCircle = 0;
    int32 MaxSheepPerCircle = 8;

    for (int32 i = 0; i < SheepEntities.Num(); ++i)
    {
        FMassEntityHandle SheepEntity = SheepEntities[i];

        // Calculer position en spirale
        float Angle = AngleStep * SheepInCurrentCircle;
        float RadAngle = FMath::DegreesToRadians(Angle);

        FVector Offset;
        Offset.X = FMath::Cos(RadAngle) * CurrentRadius;
        Offset.Y = FMath::Sin(RadAngle) * CurrentRadius;
        Offset.Z = 500.f;  // Spawner haut temporairement

        FVector SpawnLocation = ManagerLocation + Offset;

        // LineTrace pour trouver le sol sous la position de spawn
        FHitResult HitResult;
        FVector TraceStart = SpawnLocation + FVector(0, 0, 500.f);
        FVector TraceEnd = SpawnLocation - FVector(0, 0, 1000.f);

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);

        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
        {
            // Sol trouvé, placer le mouton dessus
            SpawnLocation.Z = HitResult.Location.Z + MeshHeightOffset;
        }
        else
        {
            // Pas de sol trouvé, utiliser la hauteur du manager
            SpawnLocation.Z = ManagerLocation.Z + MeshHeightOffset;
        }

        // Prochaine position dans la spirale
        SheepInCurrentCircle++;
        if (SheepInCurrentCircle >= MaxSheepPerCircle)
        {
            SheepInCurrentCircle = 0;
            CurrentRadius += 150.f;
            MaxSheepPerCircle += 4;
        }

        FTransformFragment& TransformFrag = EntityManager.GetFragmentDataChecked<FTransformFragment>(SheepEntity);
        TransformFrag.GetMutableTransform().SetLocation(SpawnLocation);
        TransformFrag.GetMutableTransform().SetRotation(FQuat::Identity);
        TransformFrag.GetMutableTransform().SetScale3D(FVector(1.f));

        FSheepStateFragment& SheepState = EntityManager.GetFragmentDataChecked<FSheepStateFragment>(SheepEntity);
        SheepState.CurrentState = ESheepState::Grazing;
        SheepState.CurrentSpeed = 0.f;
        SheepState.StateTimer = FMath::FRandRange(0.f, 3.f);
        SheepState.MovementDirection = FVector::ZeroVector;
        SheepState.GroundZ = SpawnLocation.Z + MeshHeightOffset;

        // === VARIÉTÉ : Taille aléatoire ===
        if (bEnableSizeVariety)
        {
            SheepState.Scale = FMath::FRandRange(MinScale, MaxScale);
        }
        else
        {
            SheepState.Scale = 1.0f;
        }

        // Appliquer la taille à l'instance
        FTransform InstanceTransform = TransformFrag.GetTransform();
        InstanceTransform.SetScale3D(FVector(SheepState.Scale));
        SheepInstances->AddInstance(InstanceTransform, true);
    }

    UE_LOG(LogTemp, Log, TEXT("MassSheepManager: Successfully initialized %d sheep in flock formation"), SheepEntities.Num());
}

void AMassSheepManager::UpdateSheepAI(float DeltaTime)
{
    if (!MassEntitySubsystem || SheepEntities.Num() == 0)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector DogLocation = PlayerPawn->GetActorLocation();
    FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();

    // ÉTAPE 1 : Calculer le centre du troupeau
    FVector FlockCenter = FVector::ZeroVector;
    int32 ValidSheep = 0;

    for (const FMassEntityHandle& SheepEntity : SheepEntities)
    {
        if (!EntityManager.IsEntityValid(SheepEntity))
            continue;

        FTransformFragment* TransformFrag = EntityManager.GetFragmentDataPtr<FTransformFragment>(SheepEntity);
        if (TransformFrag)
        {
            FlockCenter += TransformFrag->GetTransform().GetLocation();
            ValidSheep++;
        }
    }

    if (ValidSheep > 0)
    {
        FlockCenter /= ValidSheep;
    }

    // ÉTAPE 2 : Mettre à jour chaque mouton
    for (int32 i = 0; i < SheepEntities.Num(); ++i)
    {
        FMassEntityHandle SheepEntity = SheepEntities[i];

        if (!EntityManager.IsEntityValid(SheepEntity))
            continue;

        FSheepStateFragment* SheepState = EntityManager.GetFragmentDataPtr<FSheepStateFragment>(SheepEntity);
        FTransformFragment* TransformFrag = EntityManager.GetFragmentDataPtr<FTransformFragment>(SheepEntity);

        if (!SheepState || !TransformFrag)
            continue;

        FTransform& Transform = TransformFrag->GetMutableTransform();
        FVector SheepLocation = Transform.GetLocation();
        float DistanceToDog = FVector::Dist(SheepLocation, DogLocation);

        SheepState->DistanceToPlayer = DistanceToDog;
        SheepState->FlockCenter = FlockCenter;
        SheepState->StateTimer -= DeltaTime;

        // === COMPORTEMENT DE TROUPEAU ===

        FVector CohesionForce = FVector::ZeroVector;
        FVector SeparationForce = FVector::ZeroVector;
        FVector AlignmentForce = FVector::ZeroVector;
        int32 NearbyCount = 0;
        int32 TooCloseCount = 0;

        // Calculer les forces de troupeau
        for (const FMassEntityHandle& OtherSheep : SheepEntities)
        {
            if (OtherSheep == SheepEntity || !EntityManager.IsEntityValid(OtherSheep))
                continue;

            FTransformFragment* OtherTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(OtherSheep);
            FSheepStateFragment* OtherState = EntityManager.GetFragmentDataPtr<FSheepStateFragment>(OtherSheep);

            if (!OtherTransform || !OtherState)
                continue;

            FVector OtherLocation = OtherTransform->GetTransform().GetLocation();
            float Distance = FVector::Dist(SheepLocation, OtherLocation);

            // Cohésion : Attirer vers les moutons proches
            if (Distance < CohesionRadius && Distance > 0.1f)
            {
                CohesionForce += (OtherLocation - SheepLocation).GetSafeNormal();
                NearbyCount++;
            }

            // Séparation : S'éloigner des moutons trop proches (PRIORITAIRE)
            if (Distance < SeparationRadius && Distance > 0.1f)
            {
                // Plus on est proche, plus la force est forte (inverse carré)
                float RepulsionStrength = (SeparationRadius - Distance) / Distance;
                SeparationForce += (SheepLocation - OtherLocation).GetSafeNormal() * RepulsionStrength;
                TooCloseCount++;
            }

            // Alignement : Suivre la direction des moutons proches
            if (Distance < AlignmentRadius && Distance > 0.1f)
            {
                AlignmentForce += OtherState->MovementDirection;
            }
        }

        if (NearbyCount > 0)
        {
            CohesionForce /= NearbyCount;
        }

        if (TooCloseCount > 0)
        {
            SeparationForce /= TooCloseCount;
            SeparationForce *= 3.0f; // BOOST : Séparation 3x plus forte
        }

        // === LOGIQUE DE DÉCISION SELON LA DISTANCE AU CHIEN ===

        FVector DesiredDirection = FVector::ZeroVector;
        float DesiredSpeed = 0.f;
        ESheepState NewState = SheepState->CurrentState;

        // SI LE MOUTON EST DANS LA ZONE DE SÉCURITÉ : Ignorer le chien complètement!
        if (SheepState->bIsInSafeZone)
        {
            NewState = ESheepState::InSafeZone;
            SheepState->TimeInSafeZone += DeltaTime;

            // === FORCE VERS LE CENTRE : Actif UNIQUEMENT près des bords pour entrer/rester dans la zone ===
            FVector ToCenterForce = FVector::ZeroVector;

            if (bSafeZoneBoundsValid)
            {
                FVector ZoneCenter = SafeZoneBounds.GetCenter();
                FVector ZoneExtent = SafeZoneBounds.GetExtent();
                FVector ToCenter = ZoneCenter - SheepLocation;

                // Calculer la distance aux bords sur chaque axe
                float DistToEdgeX = ZoneExtent.X - FMath::Abs(SheepLocation.X - ZoneCenter.X);
                float DistToEdgeY = ZoneExtent.Y - FMath::Abs(SheepLocation.Y - ZoneCenter.Y);

                // SEULEMENT si proche du bord - pas d'attraction au centre sinon!
                if (DistToEdgeX < SafeZoneBorderMargin)
                {
                    float PushStrength = (1.0f - (DistToEdgeX / SafeZoneBorderMargin)) * SafeZoneConfinementStrength;
                    ToCenterForce.X = FMath::Sign(ToCenter.X) * PushStrength;
                }

                if (DistToEdgeY < SafeZoneBorderMargin)
                {
                    float PushStrength = (1.0f - (DistToEdgeY / SafeZoneBorderMargin)) * SafeZoneConfinementStrength;
                    ToCenterForce.Y = FMath::Sign(ToCenter.Y) * PushStrength;
                }

                // Si TRÈS proche de l'entrée (juste entré), pousser un peu vers l'intérieur
                float MinDistToEdge = FMath::Min(DistToEdgeX, DistToEdgeY);
                if (MinDistToEdge < SafeZoneBorderMargin * 0.5f && SheepState->TimeInSafeZone < 2.0f)
                {
                    // Pousser vers l'intérieur pendant les 2 premières secondes
                    ToCenterForce += ToCenter.GetSafeNormal() * 1.5f;
                }
            }

            // Comportement naturel dans la zone : Dispersion libre comme dans un enclos
            if (SheepState->StateTimer <= 0.f)
            {
                SheepState->StateTimer = FMath::FRandRange(3.f, 7.f);

                // 70% de chance de marcher, 30% de brouter sur place
                if (FMath::RandRange(0.f, 1.f) > 0.3f)
                {
                    // Marcher tranquillement dans différentes directions
                    DesiredSpeed = WalkSpeed * 0.5f;  // Vitesse modérée

                    // Direction TRÈS aléatoire pour se disperser + forte séparation
                    FVector RandomDir = FMath::VRand();
                    RandomDir.Z = 0.f;
                    // Priorité à la direction aléatoire et à la séparation, pas à la cohésion
                    DesiredDirection = RandomDir.GetSafeNormal() * 2.0f + SeparationForce * SafeZoneSeparationMultiplier + ToCenterForce;
                }
                else
                {
                    // Brouter tranquillement (presque immobile)
                    DesiredSpeed = ToCenterForce.SizeSquared() > 1.0f ? WalkSpeed * 0.3f : GrazingSpeed * 0.2f;
                    // Seulement séparation et force de confinement si nécessaire
                    DesiredDirection = SeparationForce * (SafeZoneSeparationMultiplier * 0.7f) + ToCenterForce;
                }
            }
            else
            {
                // Continuer le comportement actuel avec dispersion naturelle
                DesiredSpeed = SheepState->CurrentSpeed;
                // Moins de persistance de direction, plus de réactivité à la séparation
                DesiredDirection = SheepState->MovementDirection * 0.5f + SeparationForce * SafeZoneSeparationMultiplier + ToCenterForce;
            }
        }
        // SI HORS DE LA ZONE : Comportement normal (réagir au chien)
        else if (DistanceToDog < PanicDistance)
        {
            // PANIQUE : Fuir le chien
            NewState = ESheepState::Running;
            DesiredSpeed = RunSpeed;

            // Direction de fuite : Priorité MAXIMUM à la fuite, moins de séparation
            FVector FleeDirection = (SheepLocation - DogLocation).GetSafeNormal();
            DesiredDirection = FleeDirection * 5.0f + CohesionForce * 0.2f + SeparationForce * 1.0f;  // Séparation réduite de 4.0 à 1.0
        }
        else if (DistanceToDog < AlertDistance)
        {
            // ALERTE : Marcher en groupe, nerveux
            NewState = ESheepState::Walking;
            DesiredSpeed = WalkSpeed;

            // Direction : S'éloigner doucement du chien tout en restant groupé
            FVector AwayFromDog = (SheepLocation - DogLocation).GetSafeNormal();
            DesiredDirection = AwayFromDog * 0.5f + CohesionForce * 1.5f + AlignmentForce * 0.8f + SeparationForce * 2.5f;
        }
        else
        {
            // CALME : Brouter ou marcher lentement
            if (SheepState->StateTimer <= 0.f)
            {
                // Changer de comportement aléatoirement
                SheepState->StateTimer = FMath::FRandRange(3.f, 8.f);

                if (FMath::RandRange(0.f, 1.f) > 0.6f)
                {
                    NewState = ESheepState::Grazing;
                    DesiredSpeed = GrazingSpeed;

                    // Direction aléatoire douce
                    FVector RandomDir = FMath::VRand();
                    RandomDir.Z = 0.f;
                    DesiredDirection = RandomDir.GetSafeNormal() * 0.3f + CohesionForce * 1.0f + SeparationForce * 2.0f;
                }
                else
                {
                    NewState = ESheepState::Grazing;
                    DesiredSpeed = 0.f;  // S'arrêter pour brouter
                    DesiredDirection = SeparationForce * 2.0f;  // Même immobile, éviter collisions
                }
            }
            else
            {
                // Continuer le mouvement actuel
                DesiredSpeed = SheepState->CurrentSpeed;
                DesiredDirection = SheepState->MovementDirection * 0.8f + CohesionForce * 0.5f + SeparationForce * 2.0f;
            }
        }

        SheepState->CurrentState = NewState;
        SheepState->CurrentSpeed = DesiredSpeed;

        if (!DesiredDirection.IsNearlyZero())
        {
            // Lisser la direction pour éviter changements brusques
            if (!SheepState->PreviousDirection.IsNearlyZero())
            {
                DesiredDirection = FMath::Lerp(SheepState->PreviousDirection, DesiredDirection, 1.0f - DirectionSmoothness);
            }

            SheepState->MovementDirection = DesiredDirection.GetSafeNormal();
            SheepState->PreviousDirection = SheepState->MovementDirection;
        }

        // === DÉTECTION DE BLOCAGE ===

        bool bIsBlocked = false;

        if (SheepState->CurrentSpeed > MinMovementSpeed)
        {
            // Calculer distance parcourue depuis la dernière frame
            float DistanceMoved = FVector::Dist(SheepLocation, SheepState->PreviousLocation);
            float ExpectedDistance = SheepState->CurrentSpeed * DeltaTime;

            // Si le mouton veut bouger mais ne bouge presque pas = coincé
            if (DistanceMoved < ExpectedDistance * 0.3f)
            {
                SheepState->BlockedTimer += DeltaTime;

                if (SheepState->BlockedTimer > 0.5f)  // Bloqué depuis 0.5 secondes
                {
                    bIsBlocked = true;
                }
            }
            else
            {
                SheepState->BlockedTimer = 0.f;  // Réinitialiser si bouge bien
            }
        }

        // Sauvegarder position pour prochaine frame
        SheepState->PreviousLocation = SheepLocation;

        // === APPLIQUER LE MOUVEMENT ===

        if (SheepState->CurrentSpeed > 0.f && !SheepState->MovementDirection.IsNearlyZero())
        {
            FVector MovementVector = SheepState->MovementDirection * SheepState->CurrentSpeed * DeltaTime;

            // === DÉTECTION D'OBSTACLES DEVANT ===
            FHitResult ObstacleHit;
            FVector LookAheadStart = SheepLocation;
            FVector LookAheadEnd = SheepLocation + (SheepState->MovementDirection * ObstacleDetectionDistance);

            FCollisionQueryParams ObstacleQueryParams;
            ObstacleQueryParams.AddIgnoredActor(this);

            // SphereTrace pour détecter les obstacles (murs, boxes, etc.)
            bool bHitObstacle = GetWorld()->SweepSingleByChannel(
                ObstacleHit,
                LookAheadStart,
                LookAheadEnd,
                FQuat::Identity,
                ECC_WorldStatic, // Détecter les objets statiques
                FCollisionShape::MakeSphere(ObstacleDetectionRadius),
                ObstacleQueryParams
            );

            // Vérifier si l'obstacle touché a le tag configuré (par défaut "branche")
            bool bShouldAvoidObstacle = false;
            if (bHitObstacle && ObstacleHit.GetActor())
            {
                bShouldAvoidObstacle = ObstacleHit.GetActor()->ActorHasTag(ObstacleTagToAvoid);
            }

            if (bShouldAvoidObstacle)
            {
                // OBSTACLE DÉTECTÉ : Calculer direction de contournement
                FVector ObstacleNormal = ObstacleHit.Normal;
                ObstacleNormal.Z = 0.f; // Garder sur le plan horizontal

                // Choisir de contourner par la gauche ou la droite selon l'angle
                FVector CrossProduct = FVector::CrossProduct(SheepState->MovementDirection, ObstacleNormal);
                float TurnDirection = CrossProduct.Z > 0.f ? 1.f : -1.f;

                // Créer une nouvelle direction : mélange entre la normale de l'obstacle et un virage
                FVector RightVector = FVector::CrossProduct(SheepState->MovementDirection, FVector::UpVector);
                FVector AvoidanceDirection = (ObstacleNormal + (RightVector * TurnDirection)).GetSafeNormal();

                // Appliquer la direction d'évitement
                SheepState->MovementDirection = FMath::Lerp(
                    SheepState->MovementDirection,
                    AvoidanceDirection,
                    ObstacleAvoidanceStrength
                ).GetSafeNormal();

                MovementVector = SheepState->MovementDirection * SheepState->CurrentSpeed * DeltaTime;
            }

            // Si bloqué, augmenter la force de mouvement pour "forcer le passage"
            if (bIsBlocked)
            {
                // Boost de vitesse + direction aléatoire légère pour se débloquer
                MovementVector *= BlockedEscapeMultiplier;

                // Ajouter une petite composante aléatoire pour sortir du blocage
                FVector RandomJitter = FVector(
                    FMath::FRandRange(-50.f, 50.f),
                    FMath::FRandRange(-50.f, 50.f),
                    0.f
                );
                MovementVector += RandomJitter * DeltaTime;
            }

            FVector NewLocation = SheepLocation + MovementVector;

            // LineTrace pour suivre le terrain
            FHitResult HitResult;
            FVector TraceStart = NewLocation + FVector(0, 0, 500.f);
            FVector TraceEnd = NewLocation - FVector(0, 0, 1000.f);

            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(this);

            if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
            {
                NewLocation.Z = HitResult.Location.Z + MeshHeightOffset;
                SheepState->GroundZ = HitResult.Location.Z + MeshHeightOffset;
            }
            else
            {
                NewLocation.Z = SheepState->GroundZ;
            }

            Transform.SetLocation(NewLocation);

            // ORIENTATION : Le mouton regarde TOUJOURS vers sa direction (AMÉLIORÉ)
            // Ignorer la composante Z pour rester sur le plan horizontal
            FVector DirectionFlat = SheepState->MovementDirection;
            DirectionFlat.Z = 0.f;

            // Ne tourner que si le mouton se déplace vraiment (évite le spinning quand indécis)
            if (!DirectionFlat.IsNearlyZero() && SheepState->CurrentSpeed > MinMovementSpeed)
            {
                FRotator TargetRotation = DirectionFlat.Rotation();
                FRotator CurrentRotation = Transform.GetRotation().Rotator();

                // CRITIQUE : Forcer Pitch et Roll à 0 pour que le mouton reste DROIT
                TargetRotation.Pitch = 0.f;
                TargetRotation.Roll = 0.f;
                CurrentRotation.Pitch = 0.f;
                CurrentRotation.Roll = 0.f;

                // Appliquer l'offset de rotation pour compenser l'orientation du mesh
                TargetRotation.Yaw += MeshRotationOffset;

                // LISSER la rotation pour éviter les changements brusques (spinning)
                // Calculer la différence d'angle la plus courte
                float YawDifference = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw);

                // Si la différence est très grande (>90°), réduire la vitesse de rotation
                float RotationSpeed = FMath::Abs(YawDifference) > 90.f ? SharpTurnRotationSpeed : NormalRotationSpeed;

                // Interpoler vers la rotation cible
                FRotator SmoothedRotation = CurrentRotation;
                SmoothedRotation.Yaw += YawDifference * RotationSpeed;

                Transform.SetRotation(SmoothedRotation.Quaternion());
            }
            else
            {
                // Mouton immobile ou vitesse trop faible : garder la rotation actuelle
                FRotator CurrentRotation = Transform.GetRotation().Rotator();
                CurrentRotation.Pitch = 0.f;  // Pas de basculement avant/arrière
                CurrentRotation.Roll = 0.f;   // Pas de basculement gauche/droite
                Transform.SetRotation(CurrentRotation.Quaternion());
            }
        }
        else
        {
            // MÊME IMMOBILE : Assurer que le mouton reste parfaitement droit
            FRotator CurrentRotation = Transform.GetRotation().Rotator();
            CurrentRotation.Pitch = 0.f;  // Pas de basculement avant/arrière
            CurrentRotation.Roll = 0.f;   // Pas de basculement gauche/droite
            Transform.SetRotation(CurrentRotation.Quaternion());
        }
    }
}

void AMassSheepManager::UpdateVisualization()
{
    if (!MassEntitySubsystem || SheepEntities.Num() == 0)
    {
        return;
    }

    FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();

    for (int32 i = 0; i < SheepEntities.Num(); ++i)
    {
        FMassEntityHandle SheepEntity = SheepEntities[i];

        if (!EntityManager.IsEntityValid(SheepEntity))
            continue;

        FTransformFragment* TransformFrag = EntityManager.GetFragmentDataPtr<FTransformFragment>(SheepEntity);
        FSheepStateFragment* SheepState = EntityManager.GetFragmentDataPtr<FSheepStateFragment>(SheepEntity);

        if (TransformFrag && SheepState)
        {
            // Créer le transform avec la taille UNIQUE de ce mouton
            FTransform InstanceTransform = TransformFrag->GetTransform();
            InstanceTransform.SetScale3D(FVector(SheepState->Scale));

            SheepInstances->UpdateInstanceTransform(i, InstanceTransform, true, true, true);
        }
    }

    SheepInstances->MarkRenderStateDirty();
}

void AMassSheepManager::CheckSheepInSafeZone()
{
    // Si le système est désactivé ou pas de zone assignée
    if (!bEnableSafeZone || !SafeZoneActor || !MassEntitySubsystem || SheepEntities.Num() == 0)
    {
        SheepInSafeZoneCount = 0;
        return;
    }

    // Calculer ou récupérer les bounds de la zone
    if (!bSafeZoneBoundsValid)
    {
        // Chercher un composant de collision sur l'acteur de la zone
        TArray<UShapeComponent*> ShapeComponents;
        SafeZoneActor->GetComponents<UShapeComponent>(ShapeComponents);

        if (ShapeComponents.Num() > 0)
        {
            // Utiliser le premier composant trouvé (Box, Sphere, Capsule, etc.)
            SafeZoneBounds = ShapeComponents[0]->Bounds.GetBox();
            bSafeZoneBoundsValid = true;
        }
        else
        {
            // Pas de composant de collision trouvé
            UE_LOG(LogTemp, Warning, TEXT("MassSheepManager: SafeZoneActor n'a pas de composant de collision!"));
            return;
        }
    }

    FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();
    int32 SheepCount = 0;

    // Vérifier chaque mouton
    for (const FMassEntityHandle& SheepEntity : SheepEntities)
    {
        if (!EntityManager.IsEntityValid(SheepEntity))
            continue;

        FTransformFragment* TransformFrag = EntityManager.GetFragmentDataPtr<FTransformFragment>(SheepEntity);
        FSheepStateFragment* SheepState = EntityManager.GetFragmentDataPtr<FSheepStateFragment>(SheepEntity);

        if (TransformFrag && SheepState)
        {
            FVector SheepLocation = TransformFrag->GetTransform().GetLocation();

            // Vérifier si le mouton est ENTIÈREMENT dans la zone
            bool bWasInZone = SheepState->bIsInSafeZone;
            bool bIsNowInZone = SafeZoneBounds.IsInside(SheepLocation);

            if (bIsNowInZone)
            {
                SheepCount++;

                // Le mouton vient d'entrer dans la zone
                if (!bWasInZone)
                {
                    SheepState->bIsInSafeZone = true;
                    SheepState->TimeInSafeZone = 0.f;
                    UE_LOG(LogTemp, Log, TEXT("Mouton entré dans la zone de sécurité! Total: %d"), SheepCount);
                }
            }
            else
            {
                // Le mouton est sorti de la zone
                if (bWasInZone)
                {
                    SheepState->bIsInSafeZone = false;
                    SheepState->TimeInSafeZone = 0.f;
                    UE_LOG(LogTemp, Log, TEXT("Mouton sorti de la zone de sécurité!"));
                }
            }
        }
    }

    SheepInSafeZoneCount = SheepCount;

    // === DEBUG CONSOLE : Afficher le ratio moutons dans la zone / total ===
    if (bShowDebugLog)
    {
        TotalSheep = SheepEntities.Num();
        bool bShouldLog = false;

        // Log quand le compteur change
        if (SheepInSafeZoneCount != LastSheepInSafeZoneCount)
        {
            bShouldLog = true;
            LastSheepInSafeZoneCount = SheepInSafeZoneCount;
        }

        // Log périodique si activé
        if (DebugLogInterval > 0.f && DebugLogTimer >= DebugLogInterval)
        {
            bShouldLog = true;
            DebugLogTimer = 0.f;
        }

        if (bShouldLog)
        {
            float Percentage = TotalSheep > 0 ? (float)SheepInSafeZoneCount / TotalSheep * 100.f : 0.f;

            if (SheepInSafeZoneCount == TotalSheep)
            {
                // Tous les moutons sont dans la zone!
                UE_LOG(LogTemp, Display, TEXT("✅ TOUS LES MOUTONS SONT DANS LA ZONE! %d/%d (100%%)"),
                    SheepInSafeZoneCount, TotalSheep);
                Finished=true;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("🐑 Moutons dans la zone: %d/%d (%.1f%%)"),
                    SheepInSafeZoneCount, TotalSheep, Percentage);
            }
        }
    }
}