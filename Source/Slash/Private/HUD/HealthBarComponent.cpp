// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/HealthBarComponent.h"

#include "SlashCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/SphereComponent.h"
#include "HUD/HealthBar.h"


void UHealthBarComponent::SetHealthPercent(float Percent)
{
	if (HealthBarClass == nullptr) HealthBarClass = Cast<UHealthBar>(GetUserWidgetObject());

	if (HealthBarClass && HealthBarClass->HealthBar)
	{
		HealthBarClass->HealthBar->SetPercent(Percent);
		HealthBarClass->GhostPercent = Percent;
	}
	
}
