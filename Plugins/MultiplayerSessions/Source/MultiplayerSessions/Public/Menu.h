// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
class UButton;
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int NumPublicConnections=4,FString MatchType=FString(TEXT("FreeForAll")),FString LobbyPath=FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));           
	// UFUNCTION(BlueprintCallable)
	// void MenuTearDown();
	

protected:
	virtual  bool Initialize() override;
	//Virtual void OnLevelRemovedFromWorld(ULevel* InLevel,UWorld*InWorld) override;
	//自定义委托的回调函数
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	
	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults,bool bWasSuccessful);

	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

	
	
private:
	UPROPERTY(meta=(BindWidget))
	UButton* HostButton;

	UPROPERTY(meta=(BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	
	//这个子系统目的是处理联网会话功能
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	int NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};
	FString PathToLobby{TEXT("")};
};
