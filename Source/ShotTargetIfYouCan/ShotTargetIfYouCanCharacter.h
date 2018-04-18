// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShotTargetIfYouCanCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AShotTargetIfYouCanCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

public:
	AShotTargetIfYouCanCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimationMontage)
	class UAnimMontage* FireAnimation;

	/** AnimMontage to play each time we reload */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimationMontage)
	class UAnimMontage* ReloadAnimation;

	/** Muzzle flash particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
	class UParticleSystem* MuzzleFlash;

	/** Impact particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
	class UParticleSystem* Impact;

	/** In game user interface */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UserWidget)
	TSubclassOf<class UUserWidget> InGame;

	/** Game over user interface */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UserWidget)
	TSubclassOf<class UUserWidget> GameOver;

	/** Pause user interface */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UserWidget)
	TSubclassOf<class UUserWidget> Pause;

	/** Number of max bullet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 MaxAmmo;

	/** Number of bullet */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
	int32 Ammo;

	/** Weapon range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float WeaponRange;

	/** Weapon impulse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float ImpulseValue;

	/** Player's health */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
	float Health;

	/** Player's health */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
	int32 TotalScore;

	/** True when player get 10 scores */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
	float CriticalShowLeftTime;

	/** Muzzle flash particle system */
	class UParticleSystemComponent* MuzzleFlashComponent;

	/** Pause user interface */
	class UUserWidget* PauseInstance;

	/** Handle of shot timer */
	FTimerHandle ShotTimer;

	/** Handle of reload timer */
	FTimerHandle ReloadTimer;

	/** True when player can shot */
	bool bCanShot;

	/** True when player is reloading */
	bool bIsReloading;

public:
	/** Return to game */
	UFUNCTION(BlueprintCallable)
	void Unpause();

protected:
	/** Fire. */
	void Fire();

	/** Fire released. */
	void StopFiring();

	/** Shot once. */
	void Shot();

	/** Reload */
	void OnReload();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/** Called when reloading is end */
	void EndReload();

	/** Get player view point's location and rotation */
	void GetPlayerViewPoint(FVector& location, FRotator& rotation);

	/** Called when player wanna pause game */
	void OnPause();

	/** Pause game */
	void PauseGame();
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

