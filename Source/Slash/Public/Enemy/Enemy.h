// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Characters/CharacterBase.h"
#include "Interfaces/HitInterfaces.h"
#include "Characters/CharaterTypes.h"
#include "Enemy.generated.h"

class UPawnSensingComponent;
class AAIController;
class USphereComponent;
class UHealthBarComponent;
class UAnimMontage;
class USoundBase;
class UParticleSystem;

UCLASS()
class SLASH_API AEnemy : public ACharacterBase, public IHitInterfaces
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	void CheckPatrolTarget();
	
	void CheckCombatTarget();

	UPROPERTY()
	TObjectPtr<AAIController> EnemyController;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;

	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UHealthBarComponent> HealthBarComponent;

	bool InTargetRange(AActor* Target, float Range);

	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();

	void Die();

	virtual void Attack() override;
	virtual void PlayAttackMontage() override;

	UFUNCTION()
	void OnSeen(APawn* InPawn);

private:
 
	/**
	* Animation montages
	*/

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClassR;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClassL;

	UPROPERTY(EditAnywhere, Category = VisualEffects)
	UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, Category = VisualEffects)
	UParticleSystem* CombatParticle;
	
	UPROPERTY()
	AActor* CombatTarget;
	
	// Current patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;
 
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	float PatrolRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float AttackRadius = 175.f;

	FTimerHandle PatrolTimer;
	
	void PatrolTimerFinished();
 
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMin = 0.f;
 
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMax = 3.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPawnSensingComponent> PawnSensingComponent;

	EEnemyState EnemyState = EEnemyState::EES_Patrolling;
	
};


