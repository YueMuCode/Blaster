// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	
};




/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()


public:
	virtual  void DrawHUD() override;
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package){HUDPackage=Package;}

private:
	FHUDPackage HUDPackage;
};
