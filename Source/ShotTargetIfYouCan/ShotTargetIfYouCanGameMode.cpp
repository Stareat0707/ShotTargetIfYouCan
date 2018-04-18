// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ShotTargetIfYouCanGameMode.h"
#include "ShotTargetIfYouCanHUD.h"
#include "ShotTargetIfYouCanCharacter.h"
#include "UObject/ConstructorHelpers.h"

AShotTargetIfYouCanGameMode::AShotTargetIfYouCanGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Character/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AShotTargetIfYouCanHUD::StaticClass();
}
