// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitInterfaces.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHitInterfaces : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SLASH_API IHitInterfaces
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetHit(const FVector& ImpactPoint, AActor* SourceActor);

	
};
