// Fill out your copyright notice in the Description page of Project Settings.


#include "Ui/SessionsMenu.h"

#include "SessionManager.h"
#include "Components/Button.h"
#include "Logging/StructuredLog.h"

void USessionsMenu::SetupMenu()
{
	// TODO: Fix problem, when UI active with UI only input, no keyboard input is handled
}

bool USessionsMenu::Initialize()
{
	if(not Super::Initialize()) // Super must be called for bound widgets to initialize
	{
		return false;
	}

	if(UGameInstance* GameInstance = GetGameInstance())
	{
		SessionManager = MakeWeakObjectPtr(GameInstance->GetSubsystem<USessionManager>());

		SessionManager->OnCreateSessionComplete_Delegate.AddDynamic(this, &ThisClass::SessionManager_OnCreateSessionComplete);
		SessionManager->OnFindSessionsComplete_Delegate.AddUObject(this, &ThisClass::SessionManager_OnFindSessionComplete);
		SessionManager->OnJoinSessionComplete_Delegate.AddUObject(this, &ThisClass::SessionManager_OnJoinSessionComplete);
		SessionManager->OnDestroySessionComplete_Delegate.AddDynamic(this, &ThisClass::SessionManager_OnDestroySessionComplete);
		SessionManager->OnStartSessionComplete_Delegate.AddDynamic(this, &ThisClass::SessionManager_OnStartSessionComplete);
	}
	
	if(HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &USessionsMenu::OnHostButtonClicked);
	}
	
	if(JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &USessionsMenu::OnJoinButtonClicked);
	}
	
	return true;
}

void USessionsMenu::NativeDestruct()
{
	Super::NativeDestruct();
}

void USessionsMenu::SessionManager_OnCreateSessionComplete(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		if(UWorld* World = GetWorld())
		{
			World->ServerTravel(FString::Format(TEXT("{0}{1}"), { PathToLobby, "?listen"}));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString("Unable to create session"));
	}
}

void USessionsMenu::SessionManager_OnFindSessionComplete(TArray<FOnlineSessionSearchResult> const& SessionResults, bool bWasSuccessful)
{
	for(FOnlineSessionSearchResult const& Result : SessionResults)
	{
		FString MatchType;
		Result.Session.SessionSettings.Get(MatchTypeKey, MatchType);
		if(MatchType == MatchType_FreeForAll)
		{
			if(ULocalPlayer const* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController(); LocalPlayer && SessionManager.IsValid())
			{
				SessionManager->JoinGameSession(Result); // JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result);
			}
		}
	}
}

void USessionsMenu::SessionManager_OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result)
{
	if(IOnlineSubsystem const* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		if(SessionInterface)
		{
			FString ConnctionString;
			if(SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnctionString))
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange, FString::Format(TEXT("Connection String: {0}"), {ConnctionString}));

				if(APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
				{
					PlayerController->ClientTravel(ConnctionString, TRAVEL_Absolute);
				}
			}
		}
	}
}

void USessionsMenu::SessionManager_OnDestroySessionComplete(bool bWasSuccessful)
{
}

void USessionsMenu::SessionManager_OnStartSessionComplete(bool bWasSuccessful)
{
}

void USessionsMenu::OnHostButtonClicked()
{
	if(SessionManager.IsValid())
	{
		SessionManager->CreateGameSession(2, MatchType_FreeForAll.ToString());
	}
}

void USessionsMenu::OnJoinButtonClicked()
{
	if(SessionManager.IsValid())
	{
		SessionManager->FindGameSessions(10000);
	}
}
