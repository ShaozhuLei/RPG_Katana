// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "AIController.h"
#include "VectorTypes.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
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

	if (PawnSensingComponent) PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemy::OnSeen);

	EnemyController = Cast<AAIController>(GetController());

	MoveToTarget(PatrolTarget);
}

//到巡逻点后，前往下一个巡逻点
void AEnemy::CheckPatrolTarget()
{
	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();
		const float WaitTime = FMath::RandRange(WaitMin, WaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
	
}

//若玩家离开一定范围，则丢失目标
void AEnemy::CheckCombatTarget()
{
	if (!InTargetRange(CombatTarget, PatrolRadius))
	{
		//若丢失目标
		CombatTarget = nullptr;
		//if (HealthBarComponent) HealthBarComponent->SetVisibility(false);
		EnemyState = EEnemyState::EES_Patrolling;
		GetCharacterMovement()->MaxWalkSpeed = 130.f;
		MoveToTarget(PatrolTarget);
	}
	else if (!InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Chasing)
	{
		//在巡逻范围 但 不在攻击范围
		EnemyState = EEnemyState::EES_Chasing;
		GetCharacterMovement()->MaxWalkSpeed = 440.f;
		MoveToTarget(CombatTarget);
	}
	else if (InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Attacking)
	{
		//在攻击范围内
		EnemyState = EEnemyState::EES_Attacking;
		
	}
	
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

void AEnemy::OnSeen(APawn* InPawn)
{
	//没在巡逻的话 return
	if (EnemyState > EEnemyState::EES_Patrolling) return;
	
	if (InPawn->ActorHasTag(FName("SlashCharacter")))
	{
		EnemyState = EEnemyState::EES_Chasing;
		GetWorldTimerManager().ClearTimer(PatrolTimer);
		GetCharacterMovement()->MaxWalkSpeed = 440.f;
		CombatTarget = InPawn;
		MoveToTarget(CombatTarget);

		if (EnemyState != EEnemyState::EES_Attacking)
		{
			EnemyState = EEnemyState::EES_Chasing;
			MoveToTarget(CombatTarget);
		}
	}
		
	
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//时刻检查此时是否在巡逻
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
	if (AttributeComponent && HealthBarComponent)
	{
		AttributeComponent->ReceiveDamage(DamageAmount);
		HealthBarComponent->SetHealthPercent(AttributeComponent->ReturnPercentageHealth());
	}
	CombatTarget = EventInstigator->GetPawn();
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = 440.f;
	MoveToTarget(CombatTarget);
	
	return DamageAmount;
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

