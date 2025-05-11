// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASH_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttributeComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ReceiveDamage(float DamageAmount);
	float ReturnPercentageHealth();
	bool bIsAlive();
	void AddSouls(int32 NumberOfSouls);
	void AddGold(int32 AmountOfGold);
	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE int32 GetSouls() const { return Souls; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, Category="Actor Attribute")
	float Health;

	UPROPERTY(EditAnywhere, Category="Actor Attribute")
	float MaxHealth;

	UPROPERTY(EditAnywhere, Category="Actor Attribute")
	int32 Gold;
	
	UPROPERTY(EditAnywhere, Category="Actor Attribute")
	int32 Souls;

	UPROPERTY(EditAnywhere, Category="Actor Attribute")
	float MaxPosture;
};
