// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/HealthBar.h"

#include "Components/ProgressBar.h"

void UHealthBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	float ReturnValue = FMath::FInterpTo(GhostBar->GetPercent(), GhostPercent, InDeltaTime, 1);
	GhostBar->SetPercent(ReturnValue);
}
