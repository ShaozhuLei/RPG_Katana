// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Characters/CharaterTypes.h"
#include "SlashAnimInstance.generated.h"

class UCharacterMovementComponent;
class ASlashCharacter;
/**
 * 
 */
UCLASS()
class SLASH_API USlashAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ASlashCharacter> SlashCharacter;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	TObjectPtr<UCharacterMovementComponent> SlashMovementComponent;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	float GroundSpeed;
 
	UPROPERTY(BlueprintReadOnly, Category=Movement)
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category="Movement|Character State")
	ECharacterState CharacterState;

	UPROPERTY(BlueprintReadOnly, Category="Movement|Character State")
	EActionState ActionState;
};
