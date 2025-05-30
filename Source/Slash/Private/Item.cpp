// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

#include "SlashCharacter.h"
#include "Components/SphereComponent.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = ItemMesh;
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>("Sphere Component");
	SphereComponent->SetupAttachment(ItemMesh);

}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Implements<UPickupInterface>())
	{
		IPickupInterface::Execute_SetOverlappingItem(OtherActor, this);
	}
	
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->Implements<UPickupInterface>())
	{
		IPickupInterface::Execute_SetOverlappingItem(OtherActor, nullptr);
	}
}


// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	if (ItemState == EItemState::EIS_Hovering)
	{
		float Bobing = FMath::Sin(RunningTime * MoveSpeed) * Amplitude;
	
		FVector BobingDirection = FVector(0, 0, Bobing);

		AddActorWorldOffset(BobingDirection);
	}

}

