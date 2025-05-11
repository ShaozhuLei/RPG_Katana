// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

class UBoxComponent;
/**
 * 
 */
UCLASS()
class SLASH_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();

	void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator);
	void DisableSphereCollision();
	void AttachMeshToSocket(USceneComponent* InParent, FName InSocketName);

	FORCEINLINE UBoxComponent* GetBoxComponent(){return WeaponBox;}

	TArray<AActor*> IgnoreActors;

protected:

	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);

	bool ActorIsSameType(const AActor* OtherActor);
	void ExecuteGetHit(FHitResult& BoxHit);
	
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	TObjectPtr<UBoxComponent> WeaponBox;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> StartLocation;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> EndLocation;

private:

	void BoxTrace(FHitResult& BoxHit);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FVector BoxTraceExtent = FVector(5.f);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bShowBoxDebug = false;

	UPROPERTY(EditAnywhere, Category="Weapon Damage")
	float Damage = 50.f;
};
