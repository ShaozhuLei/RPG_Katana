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
	WeaponBox->SetupAttachment(MeshComp);

	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	

	StartLocation = CreateDefaultSubobject<USceneComponent>("Start Location");
	StartLocation->SetupAttachment(MeshComp);

	EndLocation = CreateDefaultSubobject<USceneComponent>("End Location");
	EndLocation->SetupAttachment(MeshComp);
	
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AEnemy>(OtherActor) != nullptr)
	{
		//if (OtherActor == this || OtherActor == GetOwner()) return;
	
		const FVector Start = StartLocation->GetComponentLocation();
		const FVector End = EndLocation->GetComponentLocation();

		TArray<AActor*> ActorsToIgnore;

		for (AActor* Actor : IgnoreActors)
		{
			ActorsToIgnore.AddUnique(Actor);
		}

		ActorsToIgnore.Add(GetOwner());
	
		FHitResult BoxHit;
	
		bool bIsHit = UKismetSystemLibrary::BoxTraceSingle(
			 this,
			 Start,
			 End,
			 FVector(20.f, 80.f, 20.f),
			 StartLocation->GetComponentRotation(),
			 ETraceTypeQuery::TraceTypeQuery1,
			 false,
			 ActorsToIgnore,
			 EDrawDebugTrace::ForDuration,
			 BoxHit,
			 true
		 );

		if (BoxHit.GetActor())
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, FString::Printf(TEXT("%s"), *BoxHit.GetActor()->GetName()));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("No Hit"));
		}
	
	
		if (AActor* Target = BoxHit.GetActor())
		{
			float DamageNum = UGameplayStatics::ApplyDamage(
				Target,
				Damage,
				GetInstigator()->GetController(),
				this,
				UDamageType::StaticClass());
		
		
			if (Target->Implements<UHitInterfaces>())
			{
				IHitInterfaces::Execute_GetHit(Target, BoxHit.ImpactPoint);
			}
			IgnoreActors.AddUnique(Target);
		
			CreateFields(BoxHit.ImpactPoint);
		
		}
	}
	
}


void AWeapon::AttachMeshToSocket(USceneComponent* InParent, FName InSocketName)
{
	FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
	MeshComp->AttachToComponent(InParent, AttachmentTransformRules, InSocketName);
}


void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
	SetOwner(NewOwner);
	SetInstigator(NewInstigator);
	AttachMeshToSocket(InParent, InSocketName);
	ItemState = EItemState::EIS_Equipped;
	if (SphereComponent)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}


