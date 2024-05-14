// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionManager.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

USessionManager::USessionManager() :
	OnCreateSessionComplete(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::SessionManager_OnCreateSessionComplete)),
	OnFindSessionsComplete(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::SessionManager_OnFindSessionsComplete)),
	OnJoinSessionComplete(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::SessionManager_OnJoinSessionComplete)),
	OnStartSessionComplete(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::SessionManager_OnStartSessionComplete)),
	OnDestroySessionComplete(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::SessionManager_OnDestroySessionComplete))
{
	
}

void USessionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	if(IOnlineSubsystem const* OS = IOnlineSubsystem::Get())
	{
		SessionInterface = OS->GetSessionInterface();
	}
	
	OnCreateSessionComplete_Handle	= SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionComplete);
	OnFindSessionsComplete_Handle	= SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsComplete);
	OnJoinSessionComplete_Handle	= SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionComplete);
	OnStartSessionComplete_Handle	= SessionInterface->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionComplete);
	OnDestroySessionComplete_Handle	= SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionComplete);
}

void USessionManager::Deinitialize()
{
	if(SessionInterface->GetNamedSession(NAME_GameSession))
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}

	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionComplete_Handle);
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsComplete_Handle);
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionComplete_Handle);
	SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionComplete_Handle);
	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionComplete_Handle);
}

void USessionManager::CreateGameSession(int32 NumPublicConnections, FString const& MatchType)
{
	check(NumPublicConnections > 0);
	
	if(not SessionInterface.IsValid())
	{
		return;
	}

	if(SessionInterface->GetNamedSession(NAME_GameSession))
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;

		DestroyGameSession();
	}

	LatestSessionSettings = MakeShared<FOnlineSessionSettings>();
	LatestSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	LatestSessionSettings->NumPublicConnections = NumPublicConnections;
	LatestSessionSettings->bAllowJoinInProgress = true;
	LatestSessionSettings->bAllowJoinViaPresence = true;
	LatestSessionSettings->bShouldAdvertise = true;
	LatestSessionSettings->bUsesPresence = true;
	LatestSessionSettings->bUseLobbiesIfAvailable = true;
	//LatestSessionSettings->BuildUniqueId = 1; // Use to allow different builds to play with each other

	LatestSessionSettings->Set(MatchTypeKey, MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	ULocalPlayer const* Player = GetWorld()->GetFirstLocalPlayerFromController();
	if(not SessionInterface->CreateSession(*Player->GetPreferredUniqueNetId(), NAME_GameSession, *LatestSessionSettings))
	{
		OnCreateSessionComplete_Delegate.Broadcast(false); // successful case handled in callback
	}
}

void USessionManager::SessionManager_OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	OnCreateSessionComplete_Delegate.Broadcast(bWasSuccessful);
}

void USessionManager::FindGameSessions(int32 MaxSessionCount)
{
	if(not SessionInterface.IsValid())
	{
		return;
	}

	check(MaxSessionCount > 0);
	
	LastSessionSearch = MakeShared<FOnlineSessionSearch>();
	LastSessionSearch->MaxSearchResults = MaxSessionCount; // Need to be a large when using dev, as 480 used by many devs/players
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	ULocalPlayer const* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if(not SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		OnFindSessionsComplete_Delegate.Broadcast({}, false);
	}
}

void USessionManager::SessionManager_OnFindSessionsComplete(bool bWasSuccessful)
{
	if(not LastSessionSearch or LastSessionSearch->SearchResults.Num() == 0)
	{
		OnFindSessionsComplete_Delegate.Broadcast({}, false);
	}
	else
	{
		OnFindSessionsComplete_Delegate.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
	}
}

void USessionManager::JoinGameSession(FOnlineSessionSearchResult const& Result)
{
	if(not SessionInterface)
	{
		OnJoinSessionComplete_Delegate.Broadcast(EOnJoinSessionCompleteResult::Type::UnknownError);
		return;
	}

	ULocalPlayer const* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if(SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result))
	{
		OnJoinSessionComplete_Delegate.Broadcast(EOnJoinSessionCompleteResult::Type::UnknownError);
	}
}

void USessionManager::SessionManager_OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type ResultType)
{
	OnJoinSessionComplete_Delegate.Broadcast(ResultType);
}

void USessionManager::StartGameSession()
{
	if(not SessionInterface)
	{
		return;
	}

	SessionInterface->StartSession(NAME_GameSession);
}

void USessionManager::SessionManager_OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	OnStartSessionComplete_Delegate.Broadcast(bWasSuccessful);
}

void USessionManager::DestroyGameSession()
{
	if(not SessionInterface)
	{
		OnDestroySessionComplete_Delegate.Broadcast(false);
		return;
	}
	
	SessionInterface->DestroySession(NAME_GameSession);
}

void USessionManager::SessionManager_OnDestroySessionComplete(FName SessionName, bool bWasSuccesful)
{
	OnDestroySessionComplete_Delegate.Broadcast(bWasSuccesful);
	
	if(bWasSuccesful and bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateGameSession(LastNumPublicConnections, LastMatchType);
	}
}