// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverHeadWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UOverHeadWidget : public UUserWidget
{
	GENERATED_BODY()


public:
	//与蓝图中的控件关联
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* DisplayText;


	void SetDisplayText(FString TextToDisplay);

	UFUNCTION(BlueprintCallable)
	void ShowPlayNetRole(APawn* InPawn);
	
protected:
	//5.0之后移除了这个APi
	//virtual void OnLevelRemoveFromWorld() override;
	virtual void NativeDestruct() override;
};
