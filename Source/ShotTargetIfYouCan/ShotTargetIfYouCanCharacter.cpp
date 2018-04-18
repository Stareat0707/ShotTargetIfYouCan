// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ShotTargetIfYouCanCharacter.h"
#include "ShotTargetIfYouCanProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"

#include "Blueprint/UserWidget.h"
#include "Classes/Particles/ParticleSystemComponent.h"
#include "ShotTargetIfYouCanTarget.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AShotTargetIfYouCanCharacter

AShotTargetIfYouCanCharacter::AShotTargetIfYouCanCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(Mesh1P, TEXT("WeaponPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.
}

void AShotTargetIfYouCanCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("WeaponPoint"));

	// Spawn muzzle flash particle
	if (MuzzleFlash != NULL)
	{
		MuzzleFlashComponent = UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, FP_Gun, "MuzzleFlashSocket", FVector(10.f, 0.f, 0.f), FRotator(), EAttachLocation::KeepRelativeOffset);
		MuzzleFlashComponent->SetVisibility(false);
	}

	// Enable player's firing
	bCanShot = true;

	// Set player's ammo to max ammo
	Ammo = MaxAmmo;

	// Initialize total score
	TotalScore = 0;

	// Initialize health
	Health = 1.f;

	// Show in game user interface
	if(InGame != NULL)
	CreateWidget<UUserWidget>(GetWorld(), InGame)->AddToViewport();

	// Switch input mode to game only
	Cast<APlayerController>(GetController())->SetInputMode(FInputModeGameOnly());
}

void AShotTargetIfYouCanCharacter::Tick(float DeltaTime)
{
	// Decrement time that critical message show
	CriticalShowLeftTime -= DeltaTime;
	if (CriticalShowLeftTime < 0)
		CriticalShowLeftTime = 0;

	// Decrement player's health.
	// If health is below 0, game is over.
	Health -= DeltaTime / 10.f;
	if (Health < 0.f)
	{
		// Show game over user interface
		if (GameOver != NULL)
			CreateWidget<UUserWidget>(GetWorld(), GameOver)->AddToViewport();
		
		// Pause game
		PauseGame();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AShotTargetIfYouCanCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire events
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShotTargetIfYouCanCharacter::Fire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShotTargetIfYouCanCharacter::StopFiring);

	// Bind reload event
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShotTargetIfYouCanCharacter::OnReload);

	// Bind pause event
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &AShotTargetIfYouCanCharacter::OnPause);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AShotTargetIfYouCanCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShotTargetIfYouCanCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

void AShotTargetIfYouCanCharacter::Fire()
{
	float duration = 0.f;

	// If player has no ammo or player is reloading, block fire
	if (!bCanShot)
		return;

	// Try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			duration = AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

	// Set muzzle flash's visibility true
	if (MuzzleFlashComponent != NULL)
		MuzzleFlashComponent->SetVisibility(true);

	// Start shooting
	GetWorldTimerManager().SetTimer(ShotTimer, this, &AShotTargetIfYouCanCharacter::Shot, duration, true, 0.f);
}

void AShotTargetIfYouCanCharacter::StopFiring()
{
	// if player is reloading, block
	if (!bCanShot)
		return;

	// stop shooting
	GetWorldTimerManager().ClearTimer(ShotTimer);

	// Set muzzle flash's visibility false
	MuzzleFlashComponent->SetVisibility(false);

	// try and stop a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Stop(0.f);
		}
	}
}

void AShotTargetIfYouCanCharacter::Shot()
{
	// Disable shot when ammo is below 0
	if (Ammo <= 0)
	{
		StopFiring();
		bCanShot = false;
		return;
	}

	// Try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// Decrement ammo
	--Ammo;

	// Get camera's location and rotation
	FVector StartTrace;
	FRotator Rotation;
	GetPlayerViewPoint(StartTrace, Rotation);

	// Set line trace start/end point
	FVector ShootDir = Rotation.Vector();
	FVector Start, End;
	Start = ShootDir * FVector::DotProduct(ShootDir, GetActorLocation() - StartTrace) + StartTrace;
	End = ShootDir * WeaponRange + StartTrace;

	// Calculate line tracing
	FHitResult OutHit;
	bool bResult = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility);

	// Return when line tracing didn't hit
	if (!bResult)
		return;

	// Spawn impact particle
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Impact, OutHit.Location);

	// Check the target hit.
	AShotTargetIfYouCanTarget* Target = Cast<AShotTargetIfYouCanTarget>(OutHit.GetActor());
	if (Target != NULL)
	{
		// Get hit score
		int32 Score = Target->GetScore(OutHit.ImpactPoint);

		// If get 10 score, increment health and show message
		if (Score == 10)
		{
			Health = FMath::Clamp<float>(Health + 0.25, 0, 1);
			CriticalShowLeftTime = 1.f;
		}

		// Add to total score
		TotalScore += Score;
	}
	else if (OutHit.GetComponent()->IsSimulatingPhysics())
	{
		// Add impulse to hit object
		FVector Impulse = (OutHit.ImpactPoint - OutHit.TraceStart).GetSafeNormal() * ImpulseValue;
		OutHit.GetComponent()->AddImpulse(Impulse);
	}
}

void AShotTargetIfYouCanCharacter::OnReload()
{
	float duration = 0.f;

	// If is reloading now, don't reload again
	if (bIsReloading)
		return;

	// stop and block user's shot
	StopFiring();
	bCanShot = false;

	// block more reloading
	bIsReloading = true;

	// try and play a reloading animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			duration = AnimInstance->Montage_Play(ReloadAnimation, 1.f);
		}
	}

	// enable user's shot
	GetWorldTimerManager().SetTimer(ReloadTimer, this, &AShotTargetIfYouCanCharacter::EndReload, duration, false, -1.f);
}

void AShotTargetIfYouCanCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AShotTargetIfYouCanCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AShotTargetIfYouCanCharacter::EndReload()
{
	bCanShot = true;
	bIsReloading = false;
	Ammo = MaxAmmo;
	GetWorldTimerManager().ClearTimer(ReloadTimer);
}

void AShotTargetIfYouCanCharacter::GetPlayerViewPoint(FVector& location, FRotator& rotation)
{
	APlayerController* Controller = Cast<APlayerController>(GetController());
	if (Controller == NULL)
		return;

	if (Controller->PlayerCameraManager == NULL)
		return;

	location = Controller->PlayerCameraManager->GetCameraLocation();
	rotation = Controller->PlayerCameraManager->GetCameraRotation();
}

void AShotTargetIfYouCanCharacter::OnPause()
{
	// Show pause user interface
	if (Pause != NULL)
	{
		PauseInstance = CreateWidget<UUserWidget>(GetWorld(), Pause);
		PauseInstance->AddToViewport();
	}

	// Pause game
	PauseGame();
}

void AShotTargetIfYouCanCharacter::PauseGame()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	PlayerController->SetPause(true);
	PlayerController->bShowMouseCursor = true;
	PlayerController->SetInputMode(FInputModeUIOnly());
}

void AShotTargetIfYouCanCharacter::Unpause()
{
	// Remove pause user interface
	if (PauseInstance != NULL)
		PauseInstance->RemoveFromParent();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	PlayerController->SetPause(false);
	PlayerController->bShowMouseCursor = false;
	PlayerController->SetInputMode(FInputModeGameOnly());
}
