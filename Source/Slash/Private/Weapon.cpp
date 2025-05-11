// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "SlashCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Enemy/Enemy.h"
#include "Interfaces/HitInterfaces.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AWeapon::AWeapon()
{
	//PrimaryActorTick.bCanEverTick = true;
	WeaponBox = CreateDefaultSubobject<UBoxComponent>("Weapon Box");
	WeaponBox->SetupAttachment(ItemMesh);

	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	

	StartLocation = CreateDefaultSubobject<USceneComponent>("Start Location");
	StartLocation->SetupAttachment(ItemMesh);

	EndLocation = CreateDefaultSubobject<USceneComponent>("End Location");
	EndLocation->SetupAttachment(ItemMesh);
	
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}


void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this || OtherActor == GetOwner() || ActorIsSameType(OtherActor)) return;
	
	FHitResult BoxHit;
	BoxTrace(BoxHit);

	if (AActor* Target = BoxHit.GetActor())
	{
		if (ActorIsSameType(BoxHit.GetActor())) return;
		UGameplayStatics::ApplyDamage(Target,Damage,GetInstigator()->GetController(),this,UDamageType::StaticClass());
		ExecuteGetHit(BoxHit);
		CreateFields(BoxHit.ImpactPoint);
	}
}

bool AWeapon::ActorIsSameType(const AActor* OtherActor)
{
	return OtherActor->ActorHasTag("Enemy") && GetOwner()->ActorHasTag("Enemy");
}

void AWeapon::ExecuteGetHit(FHitResult& BoxHit)
{
	if (BoxHit.GetActor()->Implements<UHitInterfaces>())
	{
		IHitInterfaces::Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());
	}
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector Start = StartLocation->GetComponentLocation();
	const FVector End = EndLocation->GetComponentLocation();

	TArray<AActor*> ActorsToIgnore;

	for (AActor* Actor : IgnoreActors)
	{
		ActorsToIgnore.AddUnique(Actor);
	}

	ActorsToIgnore.Add(GetOwner());
	
	bool bIsHit = UKismetSystemLibrary::BoxTraceSingle(
		 this,
		 Start,
		 End,
		 BoxTraceExtent,
		 StartLocation->GetComponentRotation(),
		 ETraceTypeQuery::TraceTypeQuery1,
		 false,
		 ActorsToIgnore,
		 bShowBoxDebug? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		 BoxHit,
		 true
	 );
	IgnoreActors.AddUnique(BoxHit.GetActor());
}


void AWeapon::DisableSphereCollision()
{
	if (SphereComponent)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, FName InSocketName)
{
	FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(InParent, AttachmentTransformRules, InSocketName);
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
	ItemState = EItemState::EIS_Equipped;
	SetOwner(NewOwner);
	SetInstigator(NewInstigator);
	AttachMeshToSocket(InParent, InSocketName);
	DisableSphereCollision();
	
}


