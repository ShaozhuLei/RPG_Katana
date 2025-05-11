// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HitInterfaces.h"
#include "BreakableActor.generated.h"

class UCapsuleComponent;
class UGeometryCollectionComponent;

UCLASS()
class SLASH_API ABreakableActor : public AActor, public IHitInterfaces
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABreakableActor();

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* SourceActor) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGeometryCollectionComponent> GeometryCollectionComponent;
	

};
