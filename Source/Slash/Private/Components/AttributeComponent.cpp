// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AttributeComponent.h"


// Sets default values for this component's properties
UAttributeComponent::UAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UAttributeComponent::ReceiveDamage(float DamageAmount)
{
	Health -= DamageAmount;
	Health = FMath::Clamp(Health, 0.f, MaxHealth);
}

void UAttributeComponent::UseStamina(float StaminaCost)
{
	Stamina -= StaminaCost;
	Stamina = FMath::Clamp(Stamina, 0.f, MaxStamina);
}

void UAttributeComponent::RegenStamina(float DelataTime)
{
	Stamina = FMath::Clamp(Stamina += DelataTime * RegenerationRate, 0.f, MaxStamina);
}

float UAttributeComponent::ReturnPercentageHealth()
{
	return Health / MaxHealth;
}

float UAttributeComponent::GetPercentageStamina()
{
	return Stamina / MaxStamina;
}

bool UAttributeComponent::bIsAlive()
{
	return Health > 0.f;
}

void UAttributeComponent::AddSouls(int32 NumberOfSouls)
{
	Souls += NumberOfSouls;
}

void UAttributeComponent::AddGold(int32 AmountOfGold)
{
	Gold += AmountOfGold;
}

