// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	HealthBarComponent = CreateDefaultSubobject<UHealthBarComponent>("Health Bar Component");
	
	HealthBarComponent->SetupAttachment(GetRootComponent());
	

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	if (HealthBarComponent) HealthBarComponent->SetVisibility(false);
	
}

void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
	if (HitReactMontage)
	{
		PlayAnimMontage(HitReactMontage, 1, SectionName);
	}
	
}

void AEnemy::DirectionalHitReact(const FVector& ImpactPoint)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	FVector PawnLocation = PlayerPawn->GetActorLocation();
	
	const FVector Forward = GetActorForwardVector();
	const FVector HitPoint = (PawnLocation - GetActorLocation()).GetSafeNormal();

	float CosinTheta = FVector::DotProduct(Forward, HitPoint);
	//cos theta 转成 Radian
	float Theta = FMath::Acos(CosinTheta);
	//Radian To Degree
	Theta = FMath::RadiansToDegrees(Theta);
	
	FVector CrossProduct = FVector::CrossProduct(Forward, HitPoint);

	if (CrossProduct.Z < 0.0f)
	{
		Theta *= -1.f;
	}

	FName MontageSection = FName();

	if (Theta >= -45.f && Theta < 45.f)
	{
		MontageSection = "HitFront";
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		MontageSection = "HitRight";
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		MontageSection = "HitLeft";
	}
	else
	{
		MontageSection = "HitBack";
	}
	
	PlayHitReactMontage(MontageSection);
}

void AEnemy::Die()
{
	//移除对刀的Box碰撞
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(3.f);
	//计算往哪个方向"死"
	int32 MontageIndex = 1;
	float Theta = FVector::DotProduct(CombatTarget->GetActorForwardVector(), GetActorForwardVector());
	
	if (Theta >= 0)
	{
		//同方向 可播放 1 2
		MontageIndex = FMath::RandRange(1, 2);
	}else if (Theta < 0)
	{
		//异方向 播放1 2 3 4
		MontageIndex = FMath::RandRange(1, 4);
	}

	FName MontageSection = FName();

	switch (MontageIndex)
	{
		case 1:
			MontageSection = "Death1";
		break;

		case 2:
			MontageSection = "Death2";
		break;

		case 3:
			MontageSection = "Death3";
		break;

		case 4:
			MontageSection = "Death4";
		break;

		default:
			MontageSection = FName();
	};

	PlayAnimMontage(DeathMontage, 1, MontageSection);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (AttributeComponent && HealthBarComponent)
	{
		AttributeComponent->ReceiveDamage(DamageAmount);
		HealthBarComponent->SetHealthPercent(AttributeComponent->ReturnPercentageHealth());
	}
	CombatTarget = EventInstigator->GetPawn();
	
	return 0;
}



void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	if (HealthBarComponent) HealthBarComponent->SetVisibility(true);
	
	if (AttributeComponent->bIsAlive())
	{
		DirectionalHitReact(ImpactPoint);
	}
	else
	{
		Die();
	}
	
	if (HitSound && GetWorld()) UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, ImpactPoint);

	if (HitParticles && GetWorld()) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
}

