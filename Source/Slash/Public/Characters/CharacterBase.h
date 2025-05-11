// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterfaces.h"
#include "CharacterBase.generated.h"

class AWeapon;
class UAttributeComponent;

UCLASS()
class SLASH_API ACharacterBase : public ACharacter, public IHitInterfaces
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase();

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Die();
	virtual void Attack();
	virtual bool CanAttack();
	virtual bool bIsAlive();
	virtual void HandleDamage(float DamageAmount);
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* SourceActor) override;

	void PlayHitReactMontage(const FName& SectionName);
	void DirectionalHitReact(const FVector& ImpactPoint, AActor* SourceActor);
	void PlayHitSound(const FVector& ImpactPoint);
	void PlayHitParticle(const FVector& ImpactPoint);
	void PlayMontageSection(UAnimMontage* AnimMontage, const FName& SectionName);
	void PlayDodgeMontage();
	void DisableMeshCollision();
	int32 PlayRandomMontageSection(UAnimMontage* AnimMontage, const TArray<FName>& SectionNames);
	int32 PlayAttackMontage();
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> AttributeComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* DodgeMontage;

	UPROPERTY(VisibleAnywhere, Category="Combat")
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = VisualEffects)
	UParticleSystem* HitParticles;

	UPROPERTY()
	AActor* CombatTarget;
	
	UPROPERTY(EditDefaultsOnly, Category="Combat")
	float AttackMontagePlatRate = 1;

	UPROPERTY(EditAnywhere, Category="Combat")
	TArray<FName> AttackMontageSections;

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UFUNCTION(BlueprintCallable)
	virtual void RollStart();

	UFUNCTION(BlueprintCallable)
	virtual void RollEnd();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void FacingTarget(const FVector& TargetLocation);

private:
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;
};
