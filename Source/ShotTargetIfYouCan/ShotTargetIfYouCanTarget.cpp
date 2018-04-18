// Fill out your copyright notice in the Description page of Project Settings.

#include "ShotTargetIfYouCanTarget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AShotTargetIfYouCanTarget::AShotTargetIfYouCanTarget()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a StaticMeshComponent
	Target = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Target"));
	SetRootComponent(Target);
	Target->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	Target->SetRelativeScale3D(FVector(3.f, 3.f, 0.1f));

	// Create a BoxCollision
	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	Collision->SetupAttachment(Target);
	Collision->SetRelativeScale3D(FVector(1.6f, 1.6f, 0.1f));
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AShotTargetIfYouCanTarget::OnComponentBeginOverlap);
}

// Called when the game starts or when spawned
void AShotTargetIfYouCanTarget::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AShotTargetIfYouCanTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Move Target to left or right
	AddActorLocalOffset(FVector(0.f, DeltaTime * Speed * (bIsMoveRight ? 1.f : -1.f), 0.f));
}

void AShotTargetIfYouCanTarget::OnComponentBeginOverlap(class UPrimitiveComponent*, class AActor*, class UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	// Switch movement Direction
	bIsMoveRight = !bIsMoveRight;
}

int32 AShotTargetIfYouCanTarget::GetScore(FVector ImpactPoint)
{
	// Convert world location to relative location
	FVector RelativeImpactPoint = ImpactPoint - GetActorLocation();

	// Get distance of location
	float Distance = RelativeImpactPoint.Size();

	// Calculate score
	int32 Score = 10 - (int32)Distance / (int32)(5.f * (GetActorScale().X));
	return Score;
}
