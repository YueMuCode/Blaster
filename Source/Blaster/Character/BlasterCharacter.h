// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
protected:
	virtual void BeginPlay() override;
public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	
protected:
	
private:
	UPROPERTY(VisibleAnywhere,Category="Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere,Category="Camera")
	UCameraComponent* FollowCamera;

	
};
