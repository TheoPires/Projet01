// Copyright Epic Games, Inc. All Rights Reserved.

#include "Projet01GameMode.h"
#include "Projet01PlayerController.h"
#include "Projet01Character.h"
#include "UObject/ConstructorHelpers.h"

AProjet01GameMode::AProjet01GameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AProjet01PlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}