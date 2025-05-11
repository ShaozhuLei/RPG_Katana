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
class SLASH_API AEnemy : public ACharacterBase
{
	GENERATED_BODY()

public:
	AEnemy();

	/**<AActor>*/
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;

	/**<IHit Interface>*/
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* SourceActor) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void HandleDamage(float DamageAmount) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	bool InTargetRange(AActor* Target, float Range);

	void MoveToTarget(AActor* Target);
	void SpawnSoul();
	void SpawnGold();
	AActor* ChoosePatrolTarget();

	virtual void Die() override;
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void AttackEnd() override;

	UFUNCTION()
	void OnSeen(APawn* InPawn);

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	

private:
	/**AI Behavior*/
	void CheckPatrolTarget();
	void CheckCombatTarget();
	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	bool IsOutSideCombatRadius();
	bool IsOutSideAttackRadius();
	bool IsInSideAttackRadius();
	bool IsChasing();
	bool IsAttacking();
	bool IsDead();
	bool IsEngaged();
	/** 战斗 */
	void StartAttackTimer();
	void ClearAttackTimer();
	void EndPatrolTimer();
	void PatrolTimerFinished();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthBarComponent> HealthBarComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CombatSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> AttackSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPawnSensingComponent> PawnSensingComponent;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClassR;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClassL;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;
 
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;
	
	FTimerHandle AttackTimer;
	FTimerHandle PatrolTimer;
	
	UPROPERTY(EditAnywhere, Category = VisualEffects)
	UParticleSystem* CombatParticle;

	UPROPERTY(EditAnywhere)
	float CombatRadius = 700.f;

	UPROPERTY(EditAnywhere)
	float PatrolAcceptRadius = 15.f;

	UPROPERTY(EditAnywhere)
	float AttackRadius = 150.f;

	UPROPERTY()
	TObjectPtr<AAIController> EnemyController;

	UPROPERTY(EditAnywhere, Category="Combat")
	float AttackMin = 0.5;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	float AttackMax = 1.2;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float PatrollingSpeed = 135.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ChasingSpeed = 440.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMin = 0.f;
 
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMax = 3.f;

	UPROPERTY(EditDefaultsOnly, Category="Pickups")
	TSubclassOf<class ASoul> SoulClass;

	UPROPERTY(EditDefaultsOnly, Category="Pickups")
	TSubclassOf<class AGold> GoldClass;
};




