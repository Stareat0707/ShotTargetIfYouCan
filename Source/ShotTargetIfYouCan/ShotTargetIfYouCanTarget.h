// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShotTargetIfYouCanTarget.generated.h"

UCLASS()
class SHOTTARGETIFYOUCAN_API AShotTargetIfYouCanTarget : public AActor
{
	GENERATED_BODY()

	/** Target mesh */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class UStaticMeshComponent* Target;

	/** Target collision to switch movement direction */
	UPROPERTY(VisibleDefaultsOnly, Category = Collision)
	class UBoxComponent* Collision;
	
public:	
	// Sets default values for this actor's properties
	AShotTargetIfYouCanTarget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Switch movement direction */
	UFUNCTION()
	void OnComponentBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Return score for hit location */
	int32 GetScore(FVector ImpactPoint);

public:
	/** Movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float Speed;

	/** True when move to right */
	bool bIsMoveRight;
};
