// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "AIController.h"
#include "VectorTypes.h"
#include "Weapon.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Item/Soul.h"
#include "Navigation/PathFollowingComponent.h"

#include "Perception/PawnSensingComponent.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	CombatSphere = CreateDefaultSubobject<USphereComponent>("CombatSphere");
	CombatSphere->SetupAttachment(GetMesh());

	AttackSphere = CreateDefaultSubobject<USphereComponent>("AttackSphere");
	AttackSphere->SetupAttachment(GetMesh());

	HealthBarComponent = CreateDefaultSubobject<UHealthBarComponent>("Health Bar Component");
	HealthBarComponent->SetupAttachment(GetRootComponent());

	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>("SensingComponent");
	PawnSensingComponent->SightRadius = 4000.f;
	PawnSensingComponent->SetPeripheralVisionAngle(45.f);

	
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	if (HealthBarComponent) HealthBarComponent->SetVisibility(false);

	Tags.Add(FName("Enemy"));

	EnemyController = Cast<AAIController>(GetController());

	MoveToTarget(PatrolTarget);

	if (PawnSensingComponent) PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemy::OnSeen);

	UWorld* World = GetWorld();
	if (World && WeaponClassR)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClassR);
		EquippedWeapon = DefaultWeapon;
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	}

	if (World && WeaponClassL)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClassL);
		DefaultWeapon->Equip(GetMesh(), FName("LeftHandSocket"), this, this);
	}

	CombatRadius = CombatSphere->GetScaledSphereRadius();
	AttackRadius = AttackSphere->GetScaledSphereRadius();
}

bool AEnemy::CanAttack()
{
	bool bCanAttack = IsInSideAttackRadius() && !IsAttacking() && !IsDead() && !IsEngaged();
	return bCanAttack;
}

//到巡逻点后，前往下一个巡逻点
void AEnemy::CheckPatrolTarget()
{
	if (InTargetRange(PatrolTarget, PatrolAcceptRadius))
	{
		PatrolTarget = ChoosePatrolTarget();
		const float WaitTime = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

//若玩家离开一定范围，则丢失目标
void AEnemy::CheckCombatTarget()
{
	if (IsOutSideCombatRadius())
	{
		ClearAttackTimer();
		LoseInterest();
		if (!IsEngaged()) StartPatrolling();
	}
	else if (IsOutSideAttackRadius() && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged()) ChaseTarget();
	}
	else if (CanAttack())
	{
		FacingTarget(CombatTarget->GetActorLocation());
		StartAttackTimer();
	}
}


bool AEnemy::InTargetRange(AActor* Target, float Range)
{
	if (Target == nullptr) return false;
	const float DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();

	return DistanceToTarget <= Range;
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(15.f);
	EnemyController->MoveTo(MoveRequest);
}

void AEnemy::SpawnSoul()
{
	if (GetWorld() && SoulClass && AttributeComponent)
	{
		//FVector SpawnLocation(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z - 20.f);
		ASoul* SpawnedSoul = GetWorld()->SpawnActor<ASoul>(SoulClass, GetActorLocation(), GetActorRotation());
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Spawn: %s"), *SpawnedSoul->GetName()));
		if (SpawnedSoul)
		{
			SpawnedSoul->SetSouls(AttributeComponent->GetSouls());
		}
	}
}

void AEnemy::SpawnGold()
{
	
}

AActor* AEnemy::ChoosePatrolTarget()
{
	TArray<AActor*> ValidTargets;
	for (AActor* Target: PatrolTargets)
	{
		if (Target != PatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}
	const int32 NumPatrolTargets = ValidTargets.Num();
	if (NumPatrolTargets > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
		return ValidTargets[TargetSelection];
	}
	return nullptr;
}

void AEnemy::Die()
{
	//移除对刀的Box碰撞
	EnemyState = EEnemyState::EES_Dead;
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	ClearAttackTimer();
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
	SpawnSoul();
}

void AEnemy::Attack()
{
	Super::Attack();
	if (CombatTarget == nullptr) return;
	
	EnemyState = EEnemyState::EES_Engaged;
	PlayAttackMontage();
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}	


void AEnemy::OnSeen(APawn* InPawn)
{
	//没在巡逻的话 return
	const bool bShouldChaseTarget =
		EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Chasing &&
		EnemyState < EEnemyState::EES_Attacking &&
		InPawn->ActorHasTag(FName("EngageableTarget"));

	if (bShouldChaseTarget)
	{
		CombatTarget = InPawn;
		EndPatrolTimer();
		ChaseTarget();
	}
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::HideHealthBar()
{
	if (HealthBarComponent) HealthBarComponent->SetVisibility(false);
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarComponent) HealthBarComponent->SetVisibility(true);
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	HideHealthBar();
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemy::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

bool AEnemy::IsOutSideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutSideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInSideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}


bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}


void AEnemy::EndPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//时刻检查此时是否在巡逻
	if (IsDead()) return;
	
	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget();
	}
	
	
	
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();
	ChaseTarget();
	return DamageAmount;
}

void AEnemy::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	if (AttributeComponent && HealthBarComponent)
	{
		HealthBarComponent->SetHealthPercent(AttributeComponent->ReturnPercentageHealth());
	}
}


void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* SourceActor)
{
	ShowHealthBar();
	EndPatrolTimer();
	ClearAttackTimer();
	Super::GetHit_Implementation(ImpactPoint, SourceActor);
	
}

