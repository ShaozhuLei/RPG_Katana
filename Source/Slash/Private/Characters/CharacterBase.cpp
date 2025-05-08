// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterBase.h"

#include "Weapon.h"
#include "Characters/CharaterTypes.h"
#include "Components/AttributeComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>("AttributeComponent");

}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACharacterBase::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName);
	}
	
}

void ACharacterBase::DirectionalHitReact(const FVector& ImpactPoint)
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

void ACharacterBase::PlayAttackMontage()
{
}

void ACharacterBase::Attack()
{
}

void ACharacterBase::AttackEnd()
{
	
}

bool ACharacterBase::CanAttack()
{
	return false;
}

void ACharacterBase::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetBoxComponent())
	{
		EquippedWeapon->GetBoxComponent()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

