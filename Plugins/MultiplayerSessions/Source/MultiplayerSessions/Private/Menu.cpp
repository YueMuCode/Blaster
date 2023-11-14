// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"



void UMenu::MenuSetup(int PNumPublicConnections,FString PMatchType,FString LobbyPath)
{
	PathToLobby=FString::Printf(TEXT("%s?listen"),*LobbyPath);
	
	
	NumPublicConnections=PNumPublicConnections;
	MatchType=PMatchType;
	//这段代码好像是为了处理UI显示时鼠标
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable=true;
	UWorld* World=GetWorld();
	if(ensure(World))
	{
		APlayerController* PlayerController=World->GetFirstPlayerController();
		if(ensure(PlayerController))
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	//获取多人会话系统
	UGameInstance* GameInstance=GetGameInstance();
	if(ensure(GameInstance))
	{
		MultiplayerSessionsSubsystem= GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if(MultiplayerSessionsSubsystem	)//委托订阅函数
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this,&ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionComplete.AddUObject(this,&ThisClass::OnFindSession);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this,&ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this,&ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this,&ThisClass::OnStartSession);
	}
	
}

bool UMenu::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}
	if(HostButton)
	{
		HostButton->OnClicked.AddDynamic(this,&ThisClass::HostButtonClicked);
	}
	if(JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this,&ThisClass::JoinButtonClicked);
	}
	return true;
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		UWorld* World=GetWorld();
		if(ensure(World))
		{
			World->ServerTravel(PathToLobby);
		}
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.0f,FColor::Cyan,FString(TEXT("会话创建成功，正在前往大厅")));
		}
	}
	else
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.0f,FColor::Red,FString(TEXT("创建会话失败，无法前往大厅")));
		}
		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if(MultiplayerSessionsSubsystem==nullptr)
	{
		return;
	}
	for(auto Result:SessionResults)
	{
		FString SettingValue;
		Result.Session.SessionSettings.Get(FName("MatchType"),SettingValue);
		if(SettingValue==MatchType)
		{
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	if(!bWasSuccessful||SessionResults.Num()==0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem=IOnlineSubsystem::Get();
	if(Subsystem)
	{
		IOnlineSessionPtr SessionInterface=Subsystem->GetSessionInterface();
		if(SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession,Address);
			APlayerController* PlayerController=GetGameInstance()->GetFirstLocalPlayerController();
			if(PlayerController)
			{
				PlayerController->ClientTravel(Address,TRAVEL_Absolute);
			}
		}
	}
	else
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,4.0f,FColor::Orange,FString(TEXT("Subsystem为空，加入失败")));
		}
	}

	if(Result!=EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if(ensure(MultiplayerSessionsSubsystem))
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections,MatchType);
	}
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Yellow,
			FString(TEXT("Host按钮被点击了"))
		);
	}
	
}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Yellow,
			FString(TEXT("Join按钮被点击了"))
		);
	}
}

// void UMenu::MenuTearDown()
// {
// 	RemoveFromParent();
// 	UWorld* World=GetWorld();
// 	if(ensure(World))
// 	{
// 		APlayerController* PlayerController=World->GetFirstPlayerController();
// 		if(PlayerController)
// 		{
// 			FInputModeGameOnly InputModeData;
// 			PlayerController->SetInputMode(InputModeData);
// 			PlayerController->SetShowMouseCursor(false);
// 		}
// 	}
// }
