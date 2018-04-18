// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShotTargetIfYouCanHUD.generated.h"

UCLASS()
class AShotTargetIfYouCanHUD : public AHUD
{
	GENERATED_BODY()

public:
	AShotTargetIfYouCanHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

