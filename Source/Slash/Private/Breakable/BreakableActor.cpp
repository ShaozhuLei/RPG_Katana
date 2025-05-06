// Fill out your copyright notice in the Description page of Project Settings.


#include "Breakable/BreakableActor.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

// Sets default values
ABreakableActor::ABreakableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>("GC Component");
	SetRootComponent(GeometryCollectionComponent);

	GeometryCollectionComponent->SetGenerateOverlapEvents(true);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("Sphere Component");
	CapsuleComponent->SetupAttachment(GetRootComponent());
	CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	
}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint)
{
	
}

// Called when the game starts or when spawned
void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	
}



