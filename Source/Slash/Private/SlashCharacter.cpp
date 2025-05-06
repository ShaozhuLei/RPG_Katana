// Fill out your copyright notice in the Description page of Project Settings.


#include "SlashCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Item.h"
#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ASlashCharacter::ASlashCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;
 
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);
	
}

void ASlashCharacter::Move(const FInputActionValue& InputActionValue)
{
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

void ASlashCharacter::PlayAttackMontage()
{
	FName SectionName = FName();
	const int32 RandNum = FMath::RandRange(0, 1);

	if (AttackMontage)
	{
		switch (RandNum)
		{
		case 0:
			SectionName = FName("Attack1");
			break;

		case 1:
			SectionName = FName("Attack2");
			break;
		default:
			SectionName = FName();
		}
		PlayAnimMontage(AttackMontage, 0.65f, SectionName);
	}
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
	
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	if (ActionState != EActionState::EAS_Unoccupied || CharacterState == ECharacterState::ECS_Unequipped || CharacterState == ECharacterState::ECS_EquippedOnBack) return false;
	
	return true;
}

void ASlashCharacter::PlayEquipMontage(const FName& MontageName)
{
	if (EquipMontage)
	{
		PlayAnimMontage(EquipMontage, 1, MontageName);
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

void ASlashCharacter::EnableBoxCollision()
{
	if (EquippedWeapon && EquippedWeapon->GetBoxComponent())
	{
		EquippedWeapon->GetBoxComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		EquippedWeapon->IgnoreActors.Empty();
	}
}

void ASlashCharacter::DisableBoxCollision()
{
	if (EquippedWeapon && EquippedWeapon->GetBoxComponent())
	{
		EquippedWeapon->GetBoxComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyPressed, ETriggerEvent::Started, this, &ASlashCharacter::PickWeapon);
		EnhancedInputComponent->BindAction(FKeyHoldWeapon, ETriggerEvent::Started, this, &ASlashCharacter::HoldWeapon);
		EnhancedInputComponent->BindAction(BaseAttack, ETriggerEvent::Started, this, &ASlashCharacter::Attack);
	}

}

