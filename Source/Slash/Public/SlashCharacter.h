// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "Characters/CharaterTypes.h"
#include "Characters/CharacterBase.h"
#include "Interfaces/HitInterfaces.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"


class USlashOverlay;
class AWeapon;
class AItem;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class SLASH_API ASlashCharacter : public ACharacterBase, public IPickupInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASlashCharacter();

	virtual void Tick(float DeltaSeconds) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* SourceActor) override;
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> EKeyPressed;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> FKeyHoldWeapon;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> BaseAttack;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> SlashMappingContext;
	
	void Move(const FInputActionValue& InputActionValue);
	void Look(const FInputActionValue& InputActionValue);
	bool IsOccupied();
	bool HasEnoughStamina();
	void Roll(const FInputActionValue& InputActionValue);
	void PickWeapon();
	void HoldWeapon();
	virtual void Attack() override;
	virtual void Die_Implementation() override;
	virtual void Jump() override;
	virtual void RollStart() override;
	virtual void RollEnd() override;
	/**Pickup Interface*/
	virtual void SetOverlappingItem_Implementation(AItem* Item) override;
	virtual void AddSouls_Implementation(ASoul* Soul) override;
	virtual void AddGold_Implementation(AGold* Gold) override;
	/**Pickup Interface end*/
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

	UPROPERTY(EditDefaultsOnly, Category="Montages")
	TObjectPtr<UAnimMontage> EquipMontage;
	
	virtual void AttackEnd() override;
	virtual bool CanAttack() override;

	void PlayEquipMontage(const FName& MontageName);
	bool CanDisarm();
	bool CanArm();

	UFUNCTION(BlueprintCallable)
	void AttachToHand();

	UFUNCTION(BlueprintCallable)
	void AttachToBack();

	UFUNCTION(BlueprintCallable)
	void FinishedEquipping();
	
	UFUNCTION(BlueprintCallable)
	void DisableBoxCollision();

	UFUNCTION(BlueprintCallable)
	void HitReactionEnd();

private:

	void InitializeSlashOverlay();
	
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;
 
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere)
	AItem* OverlappingItem;

	UPROPERTY()
	USlashOverlay* SlashOverlay;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EActionState ActionState = EActionState::EAS_Unoccupied;

};




