// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState:uint8
{
	EWS_Initial UMETA(DisplayName="Initial State"),
	EWS_Equipped UMETA(DisplayName="Equipped"),
	EWS_Dropped UMETA(DisplayName="Dropped"),

	EWS_MAX UMETA(DisplayName="DefaultMax")
};


UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	






protected:
	UFUNCTION()
	virtual void OnSphereOverlap(
	UPrimitiveComponent*OverlapperComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
	UPrimitiveComponent*OverlapperComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
	);
	
private:
	UPROPERTY(VisibleAnywhere,Category="Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere,Category="Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing=OnRep_WeaponState,VisibleAnywhere,Category="Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();
	
	UPROPERTY(VisibleAnywhere,Category="Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere,Category="Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;


public:
	void ShowPickupWidget(bool bShowWidget);
	
	void SetWeaponState(EWeaponState State);
	
	FORCEINLINE USphereComponent*GetAreaSphere() const{return AreaSphere;};
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const {return WeaponMesh;}

	virtual void Fire(const FVector& HitTarget);

	//武器准心
	UPROPERTY(EditAnywhere,Category="Crosshairs")
	UTexture2D* CrosshairCenter;

	UPROPERTY(EditAnywhere,Category="Crosshairs")
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere,Category="Crosshairs")
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere,Category="Crosshairs")
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere,Category="Crosshairs")
	UTexture2D* CrosshairsBottom;
	
};
