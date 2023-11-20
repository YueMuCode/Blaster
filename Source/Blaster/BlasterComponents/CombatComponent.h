// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class ABlasterCharacter;//朋友类将可以访问此类的所有权限

	void EquipWeapon(class AWeapon* WeaponToEquip);//class在前面起声明的作用
protected:
	virtual void BeginPlay() override;


private:
	ABlasterCharacter* Character;
	AWeapon* EquippedWeapon;
	
};
