// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterBase.h"

#include "Weapon.h"
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

void ACharacterBase::Die()
{
	Tags.Add(FName("Dead"));
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

void ACharacterBase::DirectionalHitReact(const FVector& ImpactPoint, AActor* SourceActor)
{
	const FVector Forward = GetActorForwardVector();
	const FVector HitPoint = (SourceActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("%s"), *MontageSection.ToString()));
	PlayHitReactMontage(MontageSection);
}

void ACharacterBase::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound && GetWorld()) UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, ImpactPoint);
}

void ACharacterBase::PlayHitParticle(const FVector& ImpactPoint)
{
	if (HitParticles && GetWorld()) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
}

void ACharacterBase::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage, AttackMontagePlatRate);
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ACharacterBase::PlayDodgeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DodgeMontage)
	{
		AnimInstance->Montage_Play(DodgeMontage, 1);
	}
}

void ACharacterBase::DisableMeshCollision()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

int32 ACharacterBase::PlayRandomMontageSection(UAnimMontage* AnimMontage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() == 0) return 0;
	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	PlayMontageSection(AttackMontage, SectionNames[Selection]);
	return Selection;
}

int32 ACharacterBase::PlayAttackMontage()
{
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}

void ACharacterBase::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr;
	}
}

void ACharacterBase::AttackEnd()
{
	
}

void ACharacterBase::RollStart()
{
}

void ACharacterBase::RollEnd()
{
}

bool ACharacterBase::CanAttack()
{
	return false;
}

bool ACharacterBase::bIsAlive()
{
	return AttributeComponent->bIsAlive();
}

void ACharacterBase::HandleDamage(float DamageAmount)
{
	if (AttributeComponent)
	{
		AttributeComponent->ReceiveDamage(DamageAmount);
	}
}

void ACharacterBase::GetHit_Implementation(const FVector& ImpactPoint, AActor* SourceActor)
{
	PlayHitSound(ImpactPoint);
	PlayHitParticle(ImpactPoint);

	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (bIsAlive())
	{
		DirectionalHitReact(ImpactPoint, SourceActor);
	}
	else
	{
		Die();
	}
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

