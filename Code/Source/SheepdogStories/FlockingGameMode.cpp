// Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingGameMode.h"

#include "SheepdogCharacter.h"

AFlockingGameMode::AFlockingGameMode()
{
	DefaultPawnClass = ASheepdogCharacter::StaticClass();

	PlayerControllerClass = APlayerController::StaticClass();
}
