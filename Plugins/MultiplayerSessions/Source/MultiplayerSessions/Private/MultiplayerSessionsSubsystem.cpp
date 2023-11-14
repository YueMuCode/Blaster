// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnCreateSessionComplete)),
	FindSessionCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this,&ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this,&ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnStartSessionComplete))

{
	 IOnlineSubsystem* Subsystem=IOnlineSubsystem::Get();
	 if(Subsystem)
	 {
	 	SessionInterface=Subsystem->GetSessionInterface();
	 }
}

void UMultiplayerSessionsSubsystem::CreateSession(int NumPublicConnections, FString MatchType)
{
	if(!SessionInterface.IsValid())
	{
		return;
	}
	auto ExistingSession=SessionInterface->GetNamedSession(NAME_GameSession);
	if(ExistingSession!=nullptr)
	{
		bCreateSessionOnDestroy=true;
		LastNumPublicConnections=NumPublicConnections;
		LastMatchType=MatchType;
		DestroySession();
	}

	//用handle存储delegate，方便订阅和解除订阅
	CreateSessionCompleteDelegateHandle=SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	
	LastSessionSettings=MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch=IOnlineSubsystem::Get()->GetSubsystemName()=="NULL"?true:false;
 	LastSessionSettings->NumPublicConnections=NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress=true;
	LastSessionSettings->bAllowJoinViaPresence=true;
	LastSessionSettings->bShouldAdvertise=true;
	LastSessionSettings->bUsesPresence=true;
	LastSessionSettings->bUseLobbiesIfAvailable=true;
	LastSessionSettings->Set(FName("MatchType"),MatchType,EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->BuildUniqueId=1;//防止搜索出现重叠
	
	const ULocalPlayer* LocalPlayer=GetWorld()->GetFirstLocalPlayerFromController();
	if(!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession,*LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		//广播委托
		MultiplayerOnCreateSessionComplete.Broadcast(false);
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.0f,FColor::Green,FString(TEXT("CreateSession--尝试创建session失败")));
		}
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int MaxSearchResults)
{
	if(!SessionInterface.IsValid())
	{
		return;
	}
	FindSessionCompleteDelegateHandle=SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegate);

	LastSessionSearch=MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults=MaxSearchResults;
	LastSessionSearch->bIsLanQuery=IOnlineSubsystem::Get()->GetSubsystemName()=="NULL"?true:false;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE,true,EOnlineComparisonOp::Equals);
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if(!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(),LastSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
		MultiplayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false);
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.0f,FColor::Green,FString(TEXT("UMultiplayerSessionsSubsystem::FindSessions--查找会话失败")));
		}
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if(!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}
	JoinSessionCompleteDelegateHandle=SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if(!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession,SessionResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,4.0f,FColor::Orange,FString(TEXT("EOnJoinSessionCompleteResult::UnknownError加入失败")));
		}
	}
	
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if(!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}
	DestroySessionCompleteDelegateHandle=SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if(!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
	
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if(SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if(SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
	}
	if(LastSessionSearch->SearchResults.Num()<=0)
	{
		MultiplayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false);
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,3,FColor::Red,FString(TEXT("SearchResults.Num()<=0")));
		}
		return;
	}
	
	MultiplayerOnFindSessionComplete.Broadcast(LastSessionSearch->SearchResults,bWasSuccessful);
	if(GEngine&&!bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(-1,4.0f,FColor::Orange,FString(TEXT("UMultiplayerSessionsSubsystem::OnFindSessionsComplete查找失败")));
	}
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if(SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if(SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	if(bWasSuccessful&&bCreateSessionOnDestroy)//成功销毁
	{
		bCreateSessionOnDestroy=false;
		CreateSession(LastNumPublicConnections,LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
