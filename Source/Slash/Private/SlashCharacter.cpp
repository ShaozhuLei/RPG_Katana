// Fill out your copyright notice in the Description page of Project Settings.


#include "SlashCharacter.h"

#include "BrainComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Item.h"
#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Item/Gold.h"
#include "Item/Soul.h"

// Sets default values
ASlashCharacter::ASlashCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
 
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);
	
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* SourceActor)
{
	Super::GetHit_Implementation(ImpactPoint, SourceActor);

	ActionState = EActionState::EAS_HitReaction;
}

float ASlashCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	if (SlashOverlay && AttributeComponent) SlashOverlay->SetHealthBarPercent(AttributeComponent->ReturnPercentageHealth());
	
	return DamageAmount;
}

// Called when the game starts or when spawned
void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashMappingContext, 0);
		}
	}
	Tags.Add(FName("EngageableTarget"));
	InitializeSlashOverlay();
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	if (ActionState != EActionState::EAS_Unoccupied ||
		CharacterState == ECharacterState::ECS_Unequipped ||
		CharacterState == ECharacterState::ECS_EquippedOnBack ||
		ActionState == EActionState::EAS_HitReaction) return false;
	
	return true;
}

void ASlashCharacter::Move(const FInputActionValue& InputActionValue)
{
	if (this->ActorHasTag("Dead")) return;
	const FVector2d MovementVector = InputActionValue.Get<FVector2d>();
	const FRotator Rotation = GetController()->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	
	const FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardVector, MovementVector.Y);
	
	const FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightVector, MovementVector.X);
}

void ASlashCharacter::Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookDirection = InputActionValue.Get<FVector2D>();
	AddControllerPitchInput(LookDirection.Y);
	AddControllerYawInput(LookDirection.X);
}

void ASlashCharacter::Dodge(const FInputActionValue& InputActionValue)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;
	PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodge;
}

//按"E" 键执行函数
void ASlashCharacter::PickWeapon()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	
	if (OverlappingWeapon && CharacterState == ECharacterState::ECS_Unequipped)
	{
		OverlappingWeapon->Equip(GetMesh(), FName("Sword_socket"), this, this);
		OverlappingItem = nullptr;
		EquippedWeapon = OverlappingWeapon;
		CharacterState = ECharacterState::ECS_EquippedOnBack;
	}
}

//按"F"效果
void ASlashCharacter::HoldWeapon()
{
	if (CanArm())
	{
		if (EquippedWeapon && EquippedWeapon->IsAttachedTo(this))
		{
			PlayEquipMontage("ToHand");
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		}
	}
	else if (CanDisarm())
	{
		if (EquippedWeapon && EquippedWeapon->IsAttachedTo(this))
		{
			PlayEquipMontage("ToBack");
			CharacterState = ECharacterState::ECS_EquippedOnBack;
		}
	}
}

void ASlashCharacter::Attack()
{
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::Die()
{
	Tags.Add(FName("Dead"));
	ActionState = EActionState::EAS_Dead;
	PlayAnimMontage(DeathMontage, 1, FName("UnequipDeath"));
	DisableMeshCollision();
}

void ASlashCharacter::Jump()
{
	if (this->ActorHasTag("Dead")) return;
	Super::Jump();
}

void ASlashCharacter::RollStart()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
}

void ASlashCharacter::RollEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void ASlashCharacter::SetOverlappingItem_Implementation(AItem* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::AddSouls_Implementation(ASoul* Soul)
{
	if (AttributeComponent && SlashOverlay)
	{
		AttributeComponent->AddSouls(Soul->GetSouls());
		SlashOverlay->SetSouls(AttributeComponent->GetSouls());
	}
}

void ASlashCharacter::AddGold_Implementation(AGold* Gold)
{
	if (AttributeComponent && SlashOverlay)
	{
		AttributeComponent->AddSouls(Gold->GetGold());
		SlashOverlay->SetGold(AttributeComponent->GetGold());
	}
	
}

void ASlashCharacter::PlayEquipMontage(const FName& MontageName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(MontageName);
	}
}

bool ASlashCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied && (CharacterState == ECharacterState::ECS_EquippedOneHandedWeapon || CharacterState == ECharacterState::ECS_EquippedTwoHandedWeapon);
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState == ECharacterState::ECS_EquippedOnBack;
}

void ASlashCharacter::AttachToHand()
{
	EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
}

void ASlashCharacter::AttachToBack()
{
	EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("Sword_socket"));
}

//从背上抓取武器后 进入 待机
void ASlashCharacter::FinishedEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}


void ASlashCharacter::DisableBoxCollision()
{
	if (EquippedWeapon && EquippedWeapon->GetBoxComponent())
	{
		EquippedWeapon->GetBoxComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ASlashCharacter::HitReactionEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::InitializeSlashOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
		if (SlashHUD)
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();
			if (SlashOverlay && AttributeComponent)
			{
				SlashOverlay->SetHealthBarPercent(AttributeComponent->ReturnPercentageHealth());
				SlashOverlay->SetStaminaBarPercent(1.f);
				SlashOverlay->SetGold(0);
				SlashOverlay->SetSouls(0);
			}
		}
	}
}


// Called to bind functionality to input
void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyPressed, ETriggerEvent::Started, this, &ASlashCharacter::PickWeapon);
		EnhancedInputComponent->BindAction(FKeyHoldWeapon, ETriggerEvent::Started, this, &ASlashCharacter::HoldWeapon);
		EnhancedInputComponent->BindAction(BaseAttack, ETriggerEvent::Started, this, &ASlashCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ASlashCharacter::Dodge);
	}

}

